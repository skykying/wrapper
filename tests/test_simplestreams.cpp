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
 */

#include <multipass/simplestreams.h>

#include <gmock/gmock.h>

#include <QCoreApplication>

namespace mp = multipass;

using namespace testing;

class Simplestreams : public Test
{
protected:
    virtual void SetUp()
    {
        testDir.setPath(QCoreApplication::applicationDirPath());
#ifdef WIN32
        testDir.cdUp(); // compensate for VisualStudio putting binary in additional Debug/Release subdirectory
#endif
        testDir.cd("../tests/test_data");
    }

    QDir testDir;
};

TEST_F(Simplestreams, get_image_hash_by_alias)
{
    std::string image_hash;

    EXPECT_NO_THROW(
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));

        ss_mgr.update_ss_manifest();
        image_hash = ss_mgr.get_image_hash("xenial");
    });

    EXPECT_THAT(image_hash, Eq("1797c5c82016c1e65f4008fcf89deae3a044ef76087a9ec5b907c6d64a3609ac"));
}

TEST_F(Simplestreams, get_image_hash_by_hash)
{
    std::string image_hash;

    EXPECT_NO_THROW(
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));

        ss_mgr.update_ss_manifest();
        image_hash = ss_mgr.get_image_hash("1797c5c82016c1e65f4008fcf89deae3a044ef76087a9ec5b907c6d64a3609ac");
    });

    EXPECT_THAT(image_hash, Eq("1797c5c82016c1e65f4008fcf89deae3a044ef76087a9ec5b907c6d64a3609ac"));
}

TEST_F(Simplestreams, get_image_path_by_alias)
{
    QString path;
    std::string image_hash;

    EXPECT_NO_THROW(
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));

        ss_mgr.update_ss_manifest();
        image_hash = ss_mgr.get_image_hash("xenial");
        path = ss_mgr.download_image_by_hash(image_hash);
    });

    QString image_path(testDir.filePath("test_image.img"));

    EXPECT_THAT(path.toStdString(), Eq(image_path.toStdString()));
}

TEST_F(Simplestreams, get_image_path_by_hash)
{
    QString path;
    std::string image_hash;

    EXPECT_NO_THROW(
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));
 
        ss_mgr.update_ss_manifest();
        image_hash = ss_mgr.get_image_hash("1797c5c82016c1e65f4008fcf89deae3a044ef76087a9ec5b907c6d64a3609ac");
        path = ss_mgr.download_image_by_hash(image_hash);
    });

    QString image_path(testDir.filePath("test_image.img"));

    EXPECT_THAT(path.toStdString(), Eq(image_path.toStdString()));
}

TEST_F(Simplestreams, fail_when_bad_alias)
{
    mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));
    ss_mgr.update_ss_manifest();

    EXPECT_THROW(
    {
        std::string image_hash = ss_mgr.get_image_hash("foo");
    }, std::runtime_error);
}

TEST_F(Simplestreams, fail_when_bad_hash)
{
    mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));
    ss_mgr.update_ss_manifest();

    EXPECT_THROW(
    {
        std::string image_hash = ss_mgr.get_image_hash("1797f663952c454a655787876eb56f77784234ad5e7a9ec5b907c6d64a3609ac");
    }, std::runtime_error);
}

TEST_F(Simplestreams, fail_when_index_is_not_found)
{
    mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("bad_index.json"));

    EXPECT_THROW(
    {
        ss_mgr.update_ss_manifest();
    }, std::invalid_argument);
}

TEST_F(Simplestreams, fail_when_manifest_is_not_found)
{
    mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("evil_index.json"));

    EXPECT_THROW(
    {
        ss_mgr.update_ss_manifest();
    }, std::invalid_argument);
}
