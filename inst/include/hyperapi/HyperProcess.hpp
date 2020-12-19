//---------------------------------------------------------------------------
//
// This file is the copyrighted property of Tableau Software and is protected
// by registered patents and other applicable U.S. and international laws and
// regulations.
//
// Unlicensed use of the contents of this file is prohibited. Please refer to
// the NOTICES.txt file for further details.
//
//---------------------------------------------------------------------------

/**
 * \file
 */

#ifndef TABLEAU_HYPER_HYPERPROCESS_HPP
#define TABLEAU_HYPER_HYPERPROCESS_HPP

#include <hyperapi/Endpoint.hpp>
#include <hyperapi/impl/infra.hpp>
#include <hyperapi/hyperapi.h>

#include <chrono>
#include <new>
#include <string>
#include <unordered_map>

namespace hyperapi {

/**
 * The telemetry modes
 */
enum class Telemetry {
   /** Telemetry data will be sent to tableau to help improve the Hyper API. */
   SendUsageDataToTableau = HYPER_ENABLE_TELEMETRY,
   /** No telemetry data will be sent to tableau. */
   DoNotSendUsageDataToTableau = HYPER_DISABLE_TELEMETRY
};

/**
 * Defines a Hyper process.
 * This class starts Hyper and manages its lifetime.
 * Only one Hyper process should be running concurrently to achieve optimal performance. Multiple Hyper
 * instances will compete for resources (mainly CPU cores and memory). This will lead to an
 * overall worse performance than using a single Hyper process.
 */
class HyperProcess final {
   public:
   /**
     * Starts a Hyper process with the given parameters.
     * \param hyperPath  The path to a directory containing the Hyper binaries.
     * \param telemetry  User choice on transmitting usage data to Tableau.
     * \param userAgent  The user-agent string used by Hyper in logging and telemetry
     * \param parameters Optional parameters for starting the Hyper process. All parameter keys and values are expected to be passed in UTF-8 encoding.
     *                   The available parameters are documented
     *                   <a href="https://help.tableau.com/current/api/hyper_api/en-us/reference/sql/processsettings.html" target="_top">
     *                   in the Tableau Hyper documentation, chapter "Process Settings"</a>.
     * \throw HyperException if starting the Hyper instance fails.
     * \throw std::bad_alloc
     */
   explicit HyperProcess(
      const std::string& hyperPath,
      Telemetry telemetry,
      const std::string& userAgent = std::string(),
      const std::unordered_map<std::string, std::string>& parameters =
         std::unordered_map<std::string, std::string>());

   /**
     * Starts a Hyper process. Tries to locate the Hyper binaries by locating the
     * Hyper API shared object file. The shared object is assumed to reside in a libraries
     * directory that contains the Hyper binaries in a "hyper" subfolder.
     * \param telemetry  User choice on transmitting usage data to Tableau.
     * \param userAgent  The user-agent string used by `hyperd` in logging and telemetry
     * \param parameters Optional parameters for starting the Hyper process. All parameter keys and values are expected to be passed in UTF-8 encoding.
     *                   The available parameters are documented
     *                   <a href="https://help.tableau.com/current/api/hyper_api/en-us/reference/sql/processsettings.html" target="_top">
     *                   in the Tableau Hyper documentation, chapter "Process Settings"</a>.
     * \throw HyperException if starting the Hyper process fails.
     * \throw std::bad_alloc
     */
   explicit HyperProcess(
      Telemetry telemetry,
      const std::string& userAgent = std::string(),
      const std::unordered_map<std::string, std::string>& parameters =
         std::unordered_map<std::string, std::string>());

   /**
     * Constructs a `HyperProcess` object that does not represent a hyper process.
     * \post !isOpen()
     * \throw std::bad_alloc
     */
   HyperProcess() {}

   /**
     * Stops the Hyper process. This call blocks until Hyper is shut down.
     */
   ~HyperProcess() noexcept;

   /** Move constructor. */
   HyperProcess(HyperProcess&& other) noexcept;

   /** Move assignment operator. */
   HyperProcess& operator=(HyperProcess&& other) noexcept;

   /**
     * Shuts down the Hyper process.
     *
     * If `timeoutMs` > 0ms, wait for Hyper to shut down gracefully. If the process is still running after a timeout of `timeoutMs` milliseconds,
     * forcefully terminate the process and throw an exception.
     *
     * If `timeoutMs` < 0ms, wait indefinitely for Hyper to shut down.
     *
     * If `timeoutMs` == 0ms, immediately terminate Hyper forcefully. Does not throw if the process already exited with a non-zero exit code.
     *
     * \param timeoutMs The timeout in milliseconds
     * \throw HyperException Thrown if there was an error stopping the process, if the process was forcefully killed after the timeout, or if the process already exited with a non-zero exit code.
     * \post !isOpen()
     */
   void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(-1));

   /**
     * Returns the endpoint descriptor for the given Hyper process.
     * \pre isOpen()
     */
   Endpoint getEndpoint() const;

   /**
     * Returns whether the Hyper process is open.
     */
   bool isOpen() const noexcept;

   /**
     * Closes this Hyper process object.
     * \post !isOpen()
     */
   void close() noexcept;

   private:
   /// The underlying handle.
   hyper_instance_t* m_handle = nullptr;
   /// The used user agent
   std::string m_user_agent;

   friend class Parameters;
   friend optional<int> internal::getExitCode(hyperapi::HyperProcess&);
};
}

#include <hyperapi/impl/HyperProcess.impl.hpp>

#endif
