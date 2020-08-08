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

#ifndef TABLEAU_HYPER_ENDPOINT_HPP
#define TABLEAU_HYPER_ENDPOINT_HPP

#include <string>
#include <utility>

namespace hyperapi {

/// Describes a network endpoint at which a Hyper server is accessible.
class Endpoint final {
   public:
   /**
     * Creates an Endpoint from the given connection descriptor.
     *
     * The descriptor is expected to be a tableau IPC descriptor (e.g., tab.tcp://localhost:7483).
     * \param connectionDescriptor  The tableau IPC connection descriptor string
     * \param userAgent  A user agent string, which will be used in telemetry and logging.
     *                   Note: This string will be sent to Tableau as part of telemetry data.
     */
   Endpoint(std::string connectionDescriptor, std::string userAgent)
      : connectionDescriptor(std::move(connectionDescriptor)), userAgent(std::move(userAgent)) {
   }

   /**
     * Returns the connection descriptor.
     * \return The connection descriptor
     */
   const std::string& getConnectionDescriptor() const { return connectionDescriptor; }

   /**
     * Returns the user agent.
     * \return The user agent
     */
   const std::string& getUserAgent() const { return userAgent; }

   private:
   /// The connection descriptor
   std::string connectionDescriptor;
   /// The user agent
   std::string userAgent;
};
}

#endif
