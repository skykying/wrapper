/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Gerry Boland <gerry.boland@canonical.com>
 */

#include "hyperkit_virtual_machine_factory.h"
#include "hyperkit_virtual_machine.h"
#include <multipass/virtual_machine_description.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>

#include <unistd.h> // getuid

namespace mp = multipass;

namespace
{

auto base_dir(const QString& path)
{
    QFileInfo info{path};
    return info.absoluteDir();
}

QString make_cloud_init_image(const mp::VirtualMachineDescription& desc)
{
    using namespace std::string_literals;

    const QDir instance_dir = base_dir(desc.image.image_path);
    const QString cloud_init_iso = instance_dir.filePath("cloud-init-config.iso");

    if (QFile::exists(cloud_init_iso))
        return cloud_init_iso;

    QTemporaryDir scratch_dir;

    if (!scratch_dir.isValid())
    {
        throw std::runtime_error(qPrintable("Failed to create temporary directory: " + scratch_dir.errorString()));
    }

    QFile metadata{QDir(scratch_dir.path()).filePath("meta-data")};
    QFile userdata{QDir(scratch_dir.path()).filePath("user-data")};

    if (!metadata.open(QIODevice::WriteOnly) || !userdata.open(QIODevice::WriteOnly))
    {
        throw std::runtime_error("Failed to open files for writing");
    }

    const auto& name = desc.vm_name;

    metadata.write("instance-id: ");
    metadata.write(name.c_str());
    metadata.write("\n");
    metadata.write("local-hostname: ubuntu-multipass\n");

    metadata.close();

    YAML::Emitter userdata_emitter;

    userdata_emitter.SetIndent(4);
    userdata_emitter << desc.cloud_init_config;

    if (!userdata_emitter.good())
    {
        throw std::runtime_error("Failed to emit cloud-init config: " + userdata_emitter.GetLastError());
    }

    userdata.write("#cloud-config\n");
    userdata.write(userdata_emitter.c_str());
    userdata.write("\n");

    userdata.close();

    QProcess iso_creator;
    QStringList creator_args;

    creator_args << "makehybrid"
                 << "-iso"
                 << "-joliet"
                 << "-joliet-volume-name"
                 << "cidata"
                 << "-ov" // overwrite
                 << "-o" << cloud_init_iso << scratch_dir.path();

    iso_creator.start("hdiutil", creator_args);

    iso_creator.waitForFinished();
    qDebug() << "hdiutil" << creator_args;
    if (iso_creator.exitCode() != QProcess::NormalExit)
    {
        throw std::runtime_error(qPrintable("Call to hdiutil failed: " + iso_creator.readAllStandardError()));
    }

    metadata.remove();
    userdata.remove();

    return cloud_init_iso;
}

} // namespace

mp::VirtualMachine::UPtr
mp::HyperkitVirtualMachineFactory::create_virtual_machine(const VirtualMachineDescription& desc,
                                                          VMStatusMonitor& monitor)
{
    return std::make_unique<HyperkitVirtualMachine>(desc, make_cloud_init_image(desc), monitor);
}

mp::HyperkitVirtualMachineFactory::HyperkitVirtualMachineFactory()
{
    if (getuid())
    {
        throw std::runtime_error("multipassd needs to run as root");
    }
}

mp::FetchType mp::HyperkitVirtualMachineFactory::fetch_type()
{
    return mp::FetchType::ImageKernelAndInitrd;
}

mp::VMImage mp::HyperkitVirtualMachineFactory::prepare(const VMImage& source_image)
{
    // QCow2 Image needs to be uncompressed before hyperkit/xhyve can boot from it
    QFileInfo compressed_file(source_image.image_path);
    QString uncompressed_file =
        QString("%1/%2.qcow2").arg(compressed_file.path()).arg(compressed_file.completeBaseName());

    QStringList uncompress_args(
        {QStringLiteral("convert"), "-p", "-O", "qcow2", source_image.image_path, uncompressed_file});

    QProcess uncompress;
    qDebug() << QCoreApplication::applicationDirPath() + "/qemu-img" << uncompress_args;
    uncompress.start(QCoreApplication::applicationDirPath() + "/qemu-img", uncompress_args);
    uncompress.waitForFinished();

    if (uncompress.exitCode() != QProcess::NormalExit)
    {
        throw std::runtime_error(
            qPrintable("Decompression of image failed with error: " + uncompress.readAllStandardError()));
    }
    if (!QFile::exists(uncompressed_file))
    {
        throw std::runtime_error("Decompressed image file missing!");
    }

    QFile::remove(source_image.image_path);
    QFile::rename(uncompressed_file, source_image.image_path);

    return source_image;
}
