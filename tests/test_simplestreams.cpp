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

TEST_F(Simplestreams, get_image_path_by_alias)
{
    QString path;

    try
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));

        path = ss_mgr.download_image_by_alias("xenial");
    }
    catch (const std::runtime_error& error)
    {
        FAIL() << "Unexpected std::runtime_error";
    }

    QString image_path(testDir.filePath("test_image.img"));

    EXPECT_THAT(path.toStdString(), Eq(image_path.toStdString()));
}

TEST_F(Simplestreams, fail_when_bad_alias)
{
    try
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));

        QString path = ss_mgr.download_image_by_alias("foo");
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Could not find foo"));
    }
}

TEST_F(Simplestreams, fail_when_index_is_not_found)
{
    try
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("bad_index.json"));
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Could not retrieve Simplestreams index"));
    }
}

TEST_F(Simplestreams, fail_when_manifest_is_not_found)
{
    try
    {
        mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("evil_index.json"));
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& error)
    {
        EXPECT_EQ(error.what(), std::string("Could not retrieve Simplestreams manifest"));
    }
}
