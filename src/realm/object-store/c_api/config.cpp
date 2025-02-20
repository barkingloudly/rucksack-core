#include <realm/object-store/c_api/types.hpp>
#include <realm/object-store/c_api/util.hpp>

using namespace realm;
using namespace realm::c_api;

RLM_API realm_config_t* realm_config_new()
{
    return new realm_config_t{};
}

RLM_API const char* realm_config_get_path(const realm_config_t* config)
{
    return config->path.c_str();
}

RLM_API void realm_config_set_path(realm_config_t* config, const char* path)
{
    config->path = path;
}

RLM_API size_t realm_config_get_encryption_key(const realm_config_t* config, uint8_t* out_key)
{
    if (out_key) {
        std::copy(config->encryption_key.begin(), config->encryption_key.end(), out_key);
    }
    return config->encryption_key.size();
}

RLM_API bool realm_config_set_encryption_key(realm_config_t* config, const uint8_t* key, size_t key_size)
{
    return wrap_err([=]() {
        if (key_size != 0 && key_size != 64) {
            throw InvalidEncryptionKey();
        }

        config->encryption_key.clear();
        std::copy(key, key + key_size, std::back_inserter(config->encryption_key));
        return true;
    });
}

RLM_API realm_schema_t* realm_config_get_schema(const realm_config_t* config)
{
    return wrap_err([=]() -> realm_schema_t* {
        if (config->schema) {
            return new realm_schema_t{std::make_unique<Schema>(*config->schema)};
        }
        else {
            return nullptr;
        }
    });
}

RLM_API void realm_config_set_schema(realm_config_t* config, const realm_schema_t* schema)
{
    if (schema) {
        config->schema = *schema->ptr;
    }
    else {
        config->schema = util::none;
    }
}

RLM_API uint64_t realm_config_get_schema_version(const realm_config_t* config)
{
    return config->schema_version;
}

RLM_API void realm_config_set_schema_version(realm_config_t* config, uint64_t version)
{
    config->schema_version = version;
}

RLM_API realm_schema_mode_e realm_config_get_schema_mode(const realm_config_t* config)
{
    return to_capi(config->schema_mode);
}

RLM_API void realm_config_set_schema_mode(realm_config_t* config, realm_schema_mode_e mode)
{
    config->schema_mode = from_capi(mode);
}

RLM_API realm_schema_subset_mode_e realm_config_get_schema_subset_mode(const realm_config_t* config)
{
    return to_capi(config->schema_subset_mode);
}

RLM_API void realm_config_set_schema_subset_mode(realm_config_t* config, realm_schema_subset_mode_e subset_mode)
{
    config->schema_subset_mode = from_capi(subset_mode);
}

RLM_API void realm_config_set_migration_function(realm_config_t* config, realm_migration_func_t func,
                                                 realm_userdata_t userdata, realm_free_userdata_func_t callback)
{
    if (func) {
        auto migration_func = [=](SharedRealm old_realm, SharedRealm new_realm, Schema& schema) {
            realm_t r1{old_realm};
            realm_t r2{new_realm};
            realm_schema_t sch{&schema};
            if (!(func)(userdata, &r1, &r2, &sch)) {
                throw CallbackFailed{ErrorStorage::get_thread_local()->get_and_clear_user_code_error()};
            }
        };
        config->migration_function = std::move(migration_func);
    }
    else {
        config->migration_function = nullptr;
    }
    if (callback) {
        config->free_functions.emplace(userdata, callback);
    }
}

RLM_API void realm_config_set_data_initialization_function(realm_config_t* config,
                                                           realm_data_initialization_func_t func,
                                                           realm_userdata_t userdata,
                                                           realm_free_userdata_func_t callback)
{
    if (func) {
        auto init_func = [=](SharedRealm realm) {
            realm_t r{realm};
            if (!(func)(userdata, &r)) {
                throw CallbackFailed{ErrorStorage::get_thread_local()->get_and_clear_user_code_error()};
            }
        };
        config->initialization_function = std::move(init_func);
    }
    else {
        config->initialization_function = nullptr;
    }
    if (callback) {
        config->free_functions.emplace(userdata, callback);
    }
}

RLM_API void realm_config_set_should_compact_on_launch_function(realm_config_t* config,
                                                                realm_should_compact_on_launch_func_t func,
                                                                realm_userdata_t userdata,
                                                                realm_free_userdata_func_t callback)
{
    if (func) {
        auto should_func = [=](uint64_t total_bytes, uint64_t used_bytes) -> bool {
            auto result = func(userdata, total_bytes, used_bytes);
            if (auto user_code_error = ErrorStorage::get_thread_local()->get_and_clear_user_code_error())
                throw CallbackFailed{user_code_error};
            return result;
        };
        config->should_compact_on_launch_function = std::move(should_func);
    }
    else {
        config->should_compact_on_launch_function = nullptr;
    }
    if (callback) {
        config->free_functions.emplace(userdata, callback);
    }
}

RLM_API bool realm_config_get_disable_format_upgrade(const realm_config_t* config)
{
    return config->disable_format_upgrade;
}

RLM_API bool realm_config_needs_file_format_upgrade(const realm_config_t* config)
{
    return config->needs_file_format_upgrade();
}

RLM_API void realm_config_set_disable_format_upgrade(realm_config_t* config, bool b)
{
    config->disable_format_upgrade = b;
}

RLM_API bool realm_config_get_automatic_change_notifications(const realm_config_t* config)
{
    return config->automatic_change_notifications;
}

RLM_API void realm_config_set_automatic_change_notifications(realm_config_t* config, bool b)
{
    config->automatic_change_notifications = b;
}

RLM_API void realm_config_set_scheduler(realm_config_t* config, const realm_scheduler_t* scheduler)
{
    config->scheduler = *scheduler;
}

RLM_API uint64_t realm_config_get_max_number_of_active_versions(const realm_config_t* config)
{
    return uint64_t(config->max_number_of_active_versions);
}

RLM_API void realm_config_set_max_number_of_active_versions(realm_config_t* config, uint64_t n)
{
    config->max_number_of_active_versions = uint_fast64_t(n);
}

RLM_API void realm_config_set_in_memory(realm_config_t* realm_config, bool value) noexcept
{
    realm_config->in_memory = value;
}

RLM_API bool realm_config_get_in_memory(realm_config_t* realm_config) noexcept
{
    return realm_config->in_memory;
}

RLM_API void realm_config_set_fifo_path(realm_config_t* realm_config, const char* fifo_path)
{
    realm_config->fifo_files_fallback_path = fifo_path;
}

RLM_API const char* realm_config_get_fifo_path(realm_config_t* realm_config) noexcept
{
    return realm_config->fifo_files_fallback_path.c_str();
}

RLM_API void realm_config_set_cached(realm_config_t* realm_config, bool cached) noexcept
{
    realm_config->cache = cached;
}

RLM_API bool realm_config_get_cached(realm_config_t* realm_config) noexcept
{
    return realm_config->cache;
}

RLM_API void realm_config_set_automatic_backlink_handling(realm_config_t* realm_config,
                                                          bool enable_automatic_handling) noexcept
{
    realm_config->automatically_handle_backlinks_in_migrations = enable_automatic_handling;
}
