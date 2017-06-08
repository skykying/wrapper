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
 * Authored by: Chris Townsend <christopher.townsend@canonical.com>
 *
 */

#include <multipass/simplestreams.h>

#include <QEventLoop>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStandardPaths>

#include <memory>

namespace mp = multipass;

namespace
{
QByteArray get_request(QUrl const& url)
{
    QByteArray data;
    QEventLoop event_loop;

    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &event_loop, SLOT(quit()));

    QNetworkRequest req(url);
    auto reply = std::unique_ptr<QNetworkReply>(mgr.get(req));
    event_loop.exec();

    if (reply->error() == QNetworkReply::NoError)
    {
        data = reply->readAll();
    }
    else
    {
        qDebug() << "Failure" << reply->errorString();
    }

    return data;
}

QByteArray get_info_from_file(QString path)
{
    QByteArray data;
    QFile file(path);

    if (file.exists())
    {
        file.open(QIODevice::ReadOnly);
        data = file.readAll();
        file.close();
    }

    return data;
}

QByteArray download_ss_index(QString const& index_path)
{
    QByteArray index;

    if (index_path.startsWith("http"))
    {
        index = get_request(index_path);
    }
    else
    {
        index = get_info_from_file(index_path);
    }

    if (index.isEmpty())
        throw std::runtime_error("Could not retrieve Simplestreams index");

    return index;
}

QJsonObject get_ss_manifest(QByteArray index, QString const& base_path)
{
    QJsonParseError parse_error;
    QByteArray data;
    QJsonObject entries(QJsonDocument::fromJson(index, &parse_error).object()["index"].toObject());

    for (const QJsonValue& entry : entries)
    {
        const QJsonObject ss_entry(entry.toObject());

        if (ss_entry["datatype"] == "image-downloads")
        {
            data = get_request(QUrl(base_path + ss_entry["path"].toString()));
            break;
        }
        else if (ss_entry["datatype"] == "mock")
        {
            data = get_info_from_file(QDir(base_path).filePath(ss_entry["path"].toString()));
            break;
        }
    }

    if (data.isEmpty())
        throw std::runtime_error("Could not retrieve Simplestreams manifest");

    return QJsonDocument::fromJson(data).object();
}

bool alias_matches(QStringList aliases, QString alias)
{
    for (auto const& a : aliases)
    {
        if (a == alias)
            return true;
    }
    return false;
}
} // anonymous namespace

mp::ImageDownloader::ImageDownloader(QObject* parent)
    : QObject(parent), current_reply(nullptr), download_total(0), accept_ranges(false),
      download_size(0)
{
}

void mp::ImageDownloader::download(QUrl const& url, QDir const& save_dir, QString const& file_name)
{
    image_file_name = file_name;
    cache_dir = save_dir;

    current_request = QNetworkRequest(url);

    current_reply = manager.head(current_request);

    connect(current_reply, SIGNAL(finished()), this, SLOT(finished_head()));
    connect(current_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void mp::ImageDownloader::download()
{
    if (accept_ranges)
    {
        QByteArray range_header_value = "bytes=";
        if (download_total > 0)
        {
            range_header_value += QByteArray::number(download_total);
        }
        current_request.setRawHeader("Range", range_header_value);
    }
    current_request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    current_reply = manager.get(current_request);

    connect(current_reply, SIGNAL(finished()), this, SLOT(finished()));
    connect(current_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(download_progress(qint64, qint64)));
    connect(current_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
}

void mp::ImageDownloader::finished_head()
{
    if (current_reply->hasRawHeader("Accept-Ranges"))
    {
        QString ar = current_reply->rawHeader("Accept-Ranges");
        accept_ranges = (ar.compare("bytes", Qt::CaseInsensitive) == 0);
    }

    download_total = current_reply->header(QNetworkRequest::ContentLengthHeader).toInt();

    current_request.setRawHeader("Connection", "Keep-Alive");
    current_request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

    if (!cache_dir.exists())
    {
        cache_dir.mkpath(cache_dir.path());
    }

    file.setFileName(cache_dir.filePath(image_file_name + ".part"));

    if (!accept_ranges)
    {
        file.remove();
    }
    file.open(QIODevice::ReadWrite | QIODevice::Append);

    download();
}

void mp::ImageDownloader::finished()
{
    file.close();
    QFile::remove(cache_dir.filePath(image_file_name));
    file.rename(cache_dir.filePath(image_file_name + ".part"), cache_dir.filePath(image_file_name));
    current_reply = 0;
    emit download_complete();
}

void mp::ImageDownloader::download_progress(qint64 const& bytes_received, qint64 const& bytes_total)
{
    download_size = bytes_received;

    file.write(current_reply->readAll());

    int percentage = static_cast<int>((static_cast<float>(bytes_received) * 100.0) / static_cast<float>(bytes_total));
    qDebug() << "Downloaded" << percentage << "%";

    emit progress(percentage);
}

void mp::ImageDownloader::error(QNetworkReply::NetworkError code) { qDebug() << __FUNCTION__ << "(" << code << ")"; }

mp::SimpleStreams::SimpleStreams(QObject* parent)
    : QObject(parent), base_path("http://cloud-images.ubuntu.com/releases/"),
      index_path(base_path + "streams/v1/index.json"),
      index(download_ss_index(index_path)), ss_manifest(get_ss_manifest(index, base_path)),
      cache_dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
{
}

mp::SimpleStreams::SimpleStreams(QString const& base_path, QString const& index_file_name, QObject* parent)
    : QObject(parent), base_path(base_path), index_path(QDir(base_path).filePath(index_file_name)),
      index(download_ss_index(index_path)), ss_manifest(get_ss_manifest(index, base_path)), cache_dir(base_path)
{
}

void mp::SimpleStreams::save_ss_json_file()
{
    QFile index_file("./index.json");

    index_file.open(QIODevice::WriteOnly);
    index_file.write(index);
    index_file.close();
}

QString mp::SimpleStreams::download_image_by_alias(std::string const& alias)
{
    set_ss_image_product_info_by_alias(QString::fromStdString(alias));
    set_ss_image_path();

    QFileInfo file_info(ss_image_path);
    QString file_name = file_info.fileName();
    QString image_path = cache_dir.filePath(file_name);

    if (!QFile::exists(image_path))
    {
        QEventLoop event_loop;
        ImageDownloader image_downloader;

        QObject::connect(&image_downloader, SIGNAL(download_complete()), &event_loop, SLOT(quit()));
        image_downloader.download(QUrl(base_path + ss_image_path), cache_dir, file_name);

        event_loop.exec();
    }

    return image_path;
}

void mp::SimpleStreams::set_ss_image_product_info_by_alias(QString const& alias)
{
    for (const QJsonValue& product : ss_manifest["products"].toObject())
    {
        auto aliases = product.toObject()["aliases"].toString().split(",");

        if (alias_matches(aliases, alias) &&
            product.toObject()["arch"].toString() == "amd64")
        {
            ss_product = product.toObject();
        }
    }

    if (ss_product.isEmpty())
        throw std::runtime_error("Could not find " + alias.toStdString());
}

void mp::SimpleStreams::set_ss_image_path()
{
    QString last_pub_name;

    if (ss_product.isEmpty())
    {
        throw std::runtime_error("Cannot get SimpleStreams product info");
    }

    for (auto const& version : ss_product["versions"].toObject())
    {
        auto version_string = version.toObject()["pubname"].toString();
        if (version_string <= last_pub_name)
            continue;

        last_pub_name = version_string;
        auto items = version.toObject()["items"].toObject();
        for (auto const& item : items)
        {
            const QJsonObject meta(item.toObject());

            if (meta["ftype"].toString() == "disk1.img")
            {
                ss_image_path = meta["path"].toString();
            }
        }
    }

    if (ss_image_path.isEmpty())
    {
        throw std::runtime_error("Cannot find image path");
    }
}
