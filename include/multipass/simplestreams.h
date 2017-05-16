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

#ifndef MULTIPASS_SIMPLESTREAMS_H
#define MULTIPASS_SIMPLESTREAMS_H

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include <string>

namespace multipass
{
class ImageDownloader : public QObject
{
    Q_OBJECT

public:
    explicit ImageDownloader(QObject* parent = 0);
    virtual ~ImageDownloader() = default;

signals:
    void download_complete();
    void progress(int const& percentage);

public slots:
    void download(QUrl const& url, QDir const& save_dir, QString const& file_name);

private slots:
    void download();
    void finished_head();
    void finished();
    void download_progress(qint64 const& bytesReceived, qint64 const& bytes_total);
    void error(QNetworkReply::NetworkError code);

private:
    QString image_file_name;
    QNetworkAccessManager manager;
    QNetworkRequest current_request;
    QNetworkReply* current_reply;
    QDir cache_dir;
    QFile file;
    int download_total;
    bool accept_ranges;
    int download_size;
};

class SimpleStreams : public QObject
{
    Q_OBJECT

public:
    explicit SimpleStreams(QObject* parent = 0);
    explicit SimpleStreams(QString const& base_path, QString const& index_file_name, QObject* parent = 0);
    virtual ~SimpleStreams() = default;

    void save_ss_json_file();
    QString download_image_by_alias(std::string const& alias);

signals:
    void download_complete();
    void progress(int const& percentage);

private:
    void download_image(QUrl const& url);
    void set_ss_image_path();
    void set_ss_image_product_info_by_alias(QString const& alias);

private:
    QString base_path;
    QString index_path;
    QByteArray index;
    QJsonObject ss_manifest;
    QJsonObject ss_product;
    QString ss_image_path;
    QDir cache_dir;
};
}
#endif // MULTIPASS_SIMPLESTREAMS_H
