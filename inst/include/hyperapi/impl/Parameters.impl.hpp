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

#ifndef TABLEAU_HYPER_IMPL_PARAMETERS_IMPL_HPP
#define TABLEAU_HYPER_IMPL_PARAMETERS_IMPL_HPP

#include <hyperapi/impl/Parameters.hpp>

#include <cassert>

namespace hyperapi {
namespace internal {
inline Parameters::Parameters(hyper_parameters_t* handle) noexcept
   : m_handle(handle) {
   assert(m_handle);
}

inline Parameters::~Parameters() noexcept {
   if (m_handle)
      hyper_parameters_destroy(m_handle);
}

inline Parameters::Parameters(const Parameters& other) {
   assert(other.m_handle);

   m_handle = hyper_parameters_copy(other.m_handle);
   if (!m_handle)
      throw std::bad_alloc();
}

inline Parameters::Parameters(Parameters&& other) noexcept
   : m_handle(internal::exchange(other.m_handle, nullptr)) {
}

inline Parameters& Parameters::operator=(Parameters&& other) noexcept {
   assert(other.m_handle);

   if (&other != this) {
      if (m_handle)
         hyper_parameters_destroy(m_handle);

      m_handle = internal::exchange(other.m_handle, nullptr);
   }

   return *this;
}

inline Parameters& Parameters::operator=(const Parameters& other) {
   assert(other.m_handle);

   if (&other != this) {
      hyper_parameters_t* handle = hyper_parameters_copy(other.m_handle);
      if (!handle)
         throw std::bad_alloc();

      if (m_handle)
         hyper_parameters_destroy(m_handle);

      m_handle = handle;
   }

   return *this;
}

inline void Parameters::set(const char* key, const char* value) {
   assert(key);
   assert(value);

   if (hyper_error_t* error = hyper_parameters_set(m_handle, key, value))
      throw internal::makeHyperException(error);
}

inline Parameters createConnectionParameters(hyper_instance_t* instance) {
   hyper_parameters_t* handle;
   if (hyper_error_t* error = hyper_create_connection_parameters(instance, &handle)) {
      assert(!handle);
      throw internal::makeHyperException(error);
   }

   return Parameters(handle);
}

inline Parameters Parameters::createConnectionParameters() {
   return internal::createConnectionParameters(nullptr);
}
}
}

#endif
