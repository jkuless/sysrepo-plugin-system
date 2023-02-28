#pragma once

#include "core/types.hpp"

namespace ietf::sys {
namespace API {
    /**
     * @brief System container API.
     */
    class System {
    public:
        /**
         * @brief Get system hostname.
         *
         * @return Hostname.
         */
        static Hostname getHostname();

        /**
         * @brief Set system hostname. Throws a runtime_error if unable to set hostname.
         *
         * @param hostname Hostname.
         */
        static void setHostname(const Hostname& hostname);

        /**
         * @brief Get system timezone name from /etc/localtime.
         *
         * @return Timezone name.
         */
        static TimezoneName getTimezoneName();

        /**
         * @brief Set system timezone name. Throws a runtime_error if unable to set timezone.
         *
         * @param timezoneName Timezone name.
         */
        static void setTimezoneName(const TimezoneName& timezone_name);
    };

    /**
     * @brief System state container API.
     */
    class SystemState {
    public:
        /**
         * @brief Get platform information.
         *
         * @return Platform information.
         */
        static PlatformInfo getPlatformInfo();

        /**
         * @brief Get clock information.
         *
         * @return Clock information.
         */
        static ClockInfo getClockInfo();
    };
}
}