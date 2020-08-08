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

#ifndef TABLEAU_HYPER_PARAMETERS_HPP
#define TABLEAU_HYPER_PARAMETERS_HPP

#include <hyperapi/HyperException.hpp>
#include <hyperapi/HyperProcess.hpp>
#include <hyperapi/impl/infra.hpp>
#include <hyperapi/hyperapi.h>

#include <new>

namespace hyperapi {

class Connection;
class ParametersTest;

namespace internal {

class Parameters;
Parameters createConnectionParameters(hyper_instance_t*);
hyper_ping_status_t ping(const Parameters& parameters);
/**
 * A collection of key/value parameters.
 */
class Parameters final {
   public:
   /** Destructs this `parameters` object. */
   ~Parameters() noexcept;

   /**
     * Move constructor.
     *
     * \post After the move, `other` may no longer be used.
     */
   Parameters(Parameters&& other) noexcept;

   /**
     * Move assignment operator.
     *
     * \post After the move, `other` may no longer be used.
     */
   Parameters& operator=(Parameters&& other) noexcept;

   /**
     * Copy constructor.
     *
     * \throw std::bad_alloc  if not enough memory was available.
     */
   Parameters(const Parameters& other);

   /**
     * Copy assignment operator.
     *
     * \throw std::bad_alloc  if not enough memory was available.
     * \remark Strong exception safety.
     */
   Parameters& operator=(const Parameters& other);

   /**
     * Sets a parameter.
     *
     * \param key  The key (encoded as a UTF-8 string).
     * \param value  The value (encoded as a UTF-8 string).
     * \pre key != nullptr
     * \pre value != nullptr
     * \throw hyperapi::HyperException  if the operation failed.
     */
   void set(const char* key, const char* value);

   /**
     * Creates default parameters for a connection.
     * \return  The default parameters for the connection.
     */
   static Parameters createConnectionParameters();

   private:
   /**
     * Constructs a `parameters` object from the given `hyper_parameters_t*` handle.
     *
     * This object takes ownership of the given handle.
     *
     * \param handle  The underlying `hyper_parameters_t*` handle.
     * \pre `handle` is a valid, non-NULL pointer.
     */
   explicit Parameters(hyper_parameters_t* handle) noexcept;

   private:
   /// The underlying handle.
   hyper_parameters_t* m_handle;

   friend Parameters createConnectionParameters(hyper_instance_t*);
   friend hyper_ping_status_t ping(const Parameters&);
   friend class hyperapi::ParametersTest;
   friend class hyperapi::Connection;
};
}
}

#include <hyperapi/impl/Parameters.impl.hpp>

#endif
