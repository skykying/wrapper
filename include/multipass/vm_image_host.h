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
 * Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#ifndef MULTIPASS_VM_IMAGE_HOST_H
#define MULTIPASS_VM_IMAGE_HOST_H
namespace multipass
{
class VMImageQuery;
class VMImage;
class VMImageHost
{
public:
    virtual ~VMImageHost() = default;
    virtual VMImage fetch(const VMImageQuery& query) = 0;
    virtual void update_image_manifest() = 0;
    virtual std::string get_image_hash_for_query(std::string query_string) = 0;

protected:
    VMImageHost() = default;
    VMImageHost(const VMImageHost&) = delete;
    VMImageHost& operator=(const VMImageHost&) = delete;
};
}
#endif // MULTIPASS_VM_IMAGE_HOST_H
