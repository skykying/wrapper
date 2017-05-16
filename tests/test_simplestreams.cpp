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

TEST(Simplestreams, get_image_path_by_alias)
{
    QDir testDir(QCoreApplication::applicationDirPath());
    testDir.cd("../tests/test_data");

    mp::SimpleStreams ss_mgr(testDir.absolutePath(), QString("test_index.json"));

    QString path = ss_mgr.download_image_by_alias("xenial");

    QString image_path(testDir.filePath("test_image.img"));

    EXPECT_THAT(path.toStdString(), Eq(image_path.toStdString()));
}
