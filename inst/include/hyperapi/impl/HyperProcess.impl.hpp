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

#ifndef TABLEAU_HYPER_IMPL_HYPERPROCESS_IMPL_HPP
#define TABLEAU_HYPER_IMPL_HYPERPROCESS_IMPL_HPP

#include <chrono>
#include <hyperapi/HyperException.hpp>
#include <hyperapi/HyperProcess.hpp>
#include <string>
#include <hyperapi/hyperapi.h>

namespace hyperapi {
namespace internal {
/// A parameter that can be used to avoid having default parameters for the Hyperprocess.
static constexpr char noDefaultParametersIndicator[] = "no_default_parameters";

/**
 * \return The exit code of the process. Only present if the process did exit.
 * \pre `process.isOpen()`
 */
inline hyperapi::optional<int> getExitCode(hyperapi::HyperProcess& process) {
   HYPER_PRECONDITION(process.isOpen());
   int exitCode;
   bool exited = hyper_instance_get_exit_code(process.m_handle, &exitCode);
   return exited ? hyperapi::optional<int>{exitCode} : hyperapi::optional<int>{};
}
}

inline HyperProcess::HyperProcess(
   const std::string& hyperPath,
   Telemetry telemetry,
   const std::string& userAgent,
   const std::unordered_map<std::string, std::string>& parameters)
   : m_user_agent(userAgent) {
   bool useDefaultParams = (parameters.count(internal::noDefaultParametersIndicator) == 0);

   hyper_parameters_t* handle;
   if (hyper_error_t* e = hyper_create_instance_parameters(&handle, useDefaultParams)) {
      assert(!handle);
      throw internal::makeHyperException(e);
   }

   if (!handle)
      throw std::bad_alloc();

   for (const auto& parameter : parameters) {
      if (parameter.first == internal::noDefaultParametersIndicator)
         continue;

      if (hyper_error_t* error = hyper_parameters_set(handle, parameter.first.c_str(), parameter.second.c_str())) {
         hyper_parameters_destroy(handle);
         throw internal::makeHyperException(error);
      }
   }

   if (hyper_error_t* error =
          hyper_instance_create(hyperPath.c_str(), static_cast<hyper_telemetry_t>(telemetry), handle, &m_handle)) {
      hyper_parameters_destroy(handle);
      throw internal::makeHyperException(error);
   }

   hyper_parameters_destroy(handle);
}

inline HyperProcess::HyperProcess(
   Telemetry telemetry,
   const std::string& userAgent,
   const std::unordered_map<std::string, std::string>& parameters)
   : HyperProcess({}, telemetry, userAgent, parameters) {
}

inline HyperProcess::~HyperProcess() noexcept {
   close();
}

inline HyperProcess::HyperProcess(HyperProcess&& other) noexcept
   : m_handle(internal::exchange(other.m_handle, nullptr)) {
}

inline HyperProcess& HyperProcess::operator=(HyperProcess&& other) noexcept {
   if (&other != this) {
      if (m_handle)
         hyper_instance_close(m_handle);

      m_handle = internal::exchange(other.m_handle, nullptr);
   }

   return *this;
}

inline Endpoint HyperProcess::getEndpoint() const {
   HYPER_PRECONDITION_MSG(isOpen(), "Calling getEndpoint() on a HyperProcess that is not open");

   return Endpoint(hyper_instance_get_endpoint_descriptor(m_handle), m_user_agent);
}

inline void HyperProcess::shutdown(std::chrono::milliseconds timeoutMs) {
   if (m_handle) {
      hyper_error_t* e = hyper_instance_shutdown(m_handle, static_cast<int>(timeoutMs.count()));
      m_handle = nullptr;
      if (e) {
         throw internal::makeHyperException(e);
      }
   }
}

inline bool HyperProcess::isOpen() const noexcept {
   return m_handle != nullptr;
}

inline void HyperProcess::close() noexcept {
   if (m_handle) {
      hyper_instance_close(m_handle);
      m_handle = nullptr;
   }
}
}

#endif
