#include "plugin.hpp"
#include "core/common.hpp"
#include "core/context.hpp"

#include <sysrepo-cpp/Session.hpp>
#include <sysrepo-cpp/utils/utils.hpp>

#include <srpcpp.hpp>

// [TODO]: Try to remove dependency
#include "modules/system.hpp"
#include "modules/hostname.hpp"
#include "modules/timezone-name.hpp"
#include "modules/dns.hpp"
#include "modules/auth.hpp"
#include "sysrepo.h"

namespace sr = sysrepo;

/**
 * @brief Plugin init callback.
 *
 * @param session Plugin session.
 * @param priv Private data.
 *
 * @return Error code (SR_ERR_OK on success).
 */
int sr_plugin_init_cb(sr_session_ctx_t* session, void** priv)
{
    sr::ErrorCode error = sysrepo::ErrorCode::Ok;
    auto sess = sysrepo::wrapUnmanagedSession(session);
    auto& registry(srpc::ModuleRegistry<ietf::sys::PluginContext>::getInstance());
    auto ctx = new ietf::sys::PluginContext(sess);

    *priv = static_cast<void*>(ctx);

    // create session subscriptions
    SRPLG_LOG_INF(ctx->getPluginName(), "Creating plugin subscriptions");

    // [TODO]: Try to remove this dependency and use static variable in each module to register it
    registry.registerModule<SystemModule>(*ctx);
    registry.registerModule<HostnameModule>(*ctx);
    registry.registerModule<TimezoneModule>(*ctx);
    registry.registerModule<DnsModule>(*ctx);
    registry.registerModule<AuthModule>(*ctx);

    auto& modules = registry.getRegisteredModules();

    // for all registered modules - run system values check
    for (auto& mod : modules) {
        SRPLG_LOG_INF(ctx->getPluginName(), "Running system values check for module %s", mod->getName());
        for (auto& checker : mod->getValueCheckers()) {
            try {
                const auto status = checker->checkValues(sess);
                switch (status) {
                    case srpc::DatastoreValuesCheckStatus::Equal:
                        break;
                    case srpc::DatastoreValuesCheckStatus::Partial:
                        break;
                    case srpc::DatastoreValuesCheckStatus::NonExistant:
                        break;
                }
            } catch (const std::runtime_error& err) {
                SRPLG_LOG_INF(ctx->getPluginName(), "Failed to check system values for the following paths:");
                for (const auto& path : checker->getPaths()) {
                    SRPLG_LOG_INF(ctx->getPluginName(), "\t%s", path.c_str());
                }
            }
        }
    }

    // once all values have been checked subscriptions can normally be registered

    // get registered modules and create subscriptions
    for (auto& mod : modules) {
        SRPLG_LOG_INF(ctx->getPluginName(), "Registering operational callbacks for module %s", mod->getName());
        srpc::registerOperationalSubscriptions(sess, *ctx, mod);
        SRPLG_LOG_INF(ctx->getPluginName(), "Registering module change callbacks for module %s", mod->getName());
        srpc::registerModuleChangeSubscriptions(sess, *ctx, mod);
        SRPLG_LOG_INF(ctx->getPluginName(), "Registering RPC callbacks for module %s", mod->getName());
        srpc::registerRpcSubscriptions(sess, *ctx, mod);
        SRPLG_LOG_INF(ctx->getPluginName(), "Registered module %s", mod->getName());
    }

    SRPLG_LOG_INF("ietf-system-plugin", "Created plugin subscriptions");

    return static_cast<int>(error);
}

/**
 * @brief Plugin cleanup callback.
 *
 * @param session Plugin session.
 * @param priv Private data.
 *
 */
void sr_plugin_cleanup_cb(sr_session_ctx_t* session, void* priv)
{
    auto& registry(srpc::ModuleRegistry<ietf::sys::PluginContext>::getInstance());
    auto ctx = static_cast<ietf::sys::PluginContext*>(priv);
    const auto plugin_name = ctx->getPluginName();

    SRPLG_LOG_INF(plugin_name, "Plugin cleanup called");

    auto& modules = registry.getRegisteredModules();
    for (auto& mod : modules) {
        SRPLG_LOG_INF(ctx->getPluginName(), "Cleaning up module: %s", mod->getName());
    }

    // cleanup context manually
    delete ctx;

    SRPLG_LOG_INF(plugin_name, "Plugin cleanup finished");
}
