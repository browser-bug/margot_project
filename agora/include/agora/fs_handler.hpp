/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef MARGOT_AGORA_VIRTUAL_FS_HPP
#define MARGOT_AGORA_VIRTUAL_FS_HPP

#include <filesystem>
#include <memory>

#include "agora/fs_cluster.hpp"
#include "agora/fs_configuration.hpp"
#include "agora/fs_description.hpp"
#include "agora/fs_doe.hpp"
#include "agora/fs_observation.hpp"
#include "agora/fs_prediction.hpp"
#include "agora/fs_model.hpp"
#include "agora/plugin_configuration.hpp"

namespace agora {

class FsHandler {
public:
    static std::shared_ptr<FsHandler> get_instance(const FsConfiguration &configuration) {
        return std::shared_ptr<FsHandler>(new FsHandler(configuration));
    }

    // plugin configuration facilities
    template <PluginType T>
    void create_env_configuration(PluginConfiguration &pc);

    // description fs
    void store_description(const application_id &app_id, const margot::heel::block_model &description) {
        description_fs->store_description(app_id, description);
    }
    margot::heel::block_model load_description(const application_id &app_id) { return description_fs->load_description(app_id); }

    // prediction fs
    void store_prediction(const application_id &app_id, const margot::heel::block_model &description, const prediction_model &model) {
        prediction_fs->store_prediction(app_id, description, model);
    }
    prediction_model load_prediction(const application_id &app_id, const margot::heel::block_model &description) {
        return prediction_fs->load_prediction(app_id, description);
    }

    // model fs
    bool is_model_valid(const application_id &app_id, const std::string &metric_name) const {
        return model_fs->is_model_valid(app_id, metric_name);
    }

    // doe fs
    void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) {
        doe_fs->store_doe(app_id, description, doe);
    }
    doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) {
        return doe_fs->load_doe(app_id, description);
    }
    void update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id) {
        return doe_fs->update_doe(app_id, description, config_id);
    }
    void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) {
        return doe_fs->empty_doe_entries(app_id, description);
    }

    // observation fs
    void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) {
        observation_fs->create_observation_table(app_id, description);
    }
    void insert_observation_entry(const application_id &application_id, const std::string &client_id, const long duration_sec,
                                  const long duration_ns, const margot::heel::operating_point_model &operating_point) {
        observation_fs->insert_observation_entry(application_id, client_id, duration_sec, duration_ns, operating_point);
    }

    // cluster fs
    void store_cluster(const application_id &app_id, const margot::heel::block_model &description, const cluster_model &cluster) {
        cluster_fs->store_cluster(app_id, description, cluster);
    }
    cluster_model load_cluster(const application_id &app_id, const margot::heel::block_model &description) {
        return cluster_fs->load_cluster(app_id, description);
    }

    void erase(const application_id &app_id) {
        description_fs->erase(app_id);
        doe_fs->erase(app_id);
        prediction_fs->erase(app_id);
        model_fs->erase(app_id);
        observation_fs->erase(app_id);
        cluster_fs->erase(app_id);
    }

private:
    FsHandler(const FsConfiguration &configuration);

    void env_configuration_preamble(PluginConfiguration &pc);

    std::shared_ptr<Logger> logger;

    std::unique_ptr<FsDescription> description_fs;
    std::unique_ptr<FsDoe> doe_fs;
    std::unique_ptr<FsPrediction> prediction_fs;
    std::unique_ptr<FsModel> model_fs;
    std::unique_ptr<FsCluster> cluster_fs;
    std::unique_ptr<FsObservation> observation_fs;
};

}  // namespace agora

#endif  // MARGOT_AGORA_VIRTUAL_FS_HPP
