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
 * \file hyperapi/hyperapi.h
 * \brief Hyper client library for C applications
 */

#ifndef TABLEAU_HYPER_API_H
#define TABLEAU_HYPER_API_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

#ifndef HYPER_API
#if defined(_WIN32) // Windows, assume MSVC
#ifdef LIBHYPER_API_BUILD
#define HYPER_API
#else
#define HYPER_API __declspec(dllimport)
#endif
#else // POSIX, assume GCC or Clang
#ifdef LIBHYPER_API_BUILD
#define HYPER_API __attribute__((visibility("default")))
#else
/** Marks a symbol that is part of exported Hyper API. */
#define HYPER_API
#endif
#endif
#endif

//---------------------------------------------------------------------------

#ifndef __has_attribute
/** Replacement for `__has_attribute()` built-in. */
#define __has_attribute(x) 0
#endif

#if __has_attribute(warn_unused_result)
#define HYPER_API_NODISCARD __attribute__((warn_unused_result))
#else
/** Marks a function whose return value may not be discarded (aka. `[[nodiscard]]`). */
#define HYPER_API_NODISCARD
#endif

//---------------------------------------------------------------------------

#ifdef __cplusplus
/** `noexcept` specifier for Hyper API functions. */
#define HYPER_API_NOEXCEPT noexcept
#else
/** `noexcept` specifier for Hyper API functions. */
#define HYPER_API_NOEXCEPT
#endif

//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------

/**
 * \name Functions related to error handling
 *
 * These functions operate on error objects that provide additional information when an API call fails.
 *
 * Error objects always provide an error category, an error code, and a primary error message.
 *
 * The following additional fields may be available:
 *
 *  * Detail Message
 *  * Hint Message: Provides a suggestion on how to avoid the error.
 *  * Severity
 *  * SQL_STATE Code
 *
 * @{
 */

/**
 * An error object that provides additional information when an API call fails.
 */
typedef struct hyper_error_t hyper_error_t;

/**
 * Categories of errors reported by the API.
 */
typedef enum {
   /** Generic error category (C `errno.h` codes). */
   HYPER_ERROR_CATEGORY_GENERIC = 0,
   /** System error category (system errno codes on POSIX, `GetLastError()` codes on Windows). */
   HYPER_ERROR_CATEGORY_SYSTEM = 1,
   /** Error category for process exit codes (`int` on POSIX, `DWORD` on Windows). */
   HYPER_ERROR_CATEGORY_PROCESS_EXIT_CODE = 3,
   /** Error category for Hyper's SQLSTATE codes (`int32_t` values) */
   HYPER_ERROR_CATEGORY_SQLSTATE = 4
} hyper_error_category_t;

/**
 * An enumeration of potential severity levels of errors.
 */
typedef enum {
   /**
     * An error that caused the current command to abort.
     */
   HYPER_SEVERITY_ERROR,
   /**
     * An error that caused the current database session to abort.
     */
   HYPER_SEVERITY_FATAL,
   /**
     * An error that caused all database sessions to abort.
     */
   HYPER_SEVERITY_PANIC
} hyper_error_severity_t;

/**
 * An enumeration of information fields of errors.
 */
typedef enum {
   /// The error category.
   HYPER_ERROR_FIELD_ERROR_CATEGORY = 0,
   /// The error code.
   /// The meaning of the returned error code depends on the category of the error.
   HYPER_ERROR_FIELD_ERROR_CODE = 1,
   /** The primary human-readable error message (typically one line). Always present. */
   HYPER_ERROR_FIELD_MESSAGE = 2,
   /**
     * An optional suggestion what to do about the problem. This is intended to differ from detail in that it offers
     * advice (potentially inappropriate) rather than hard facts. Might run to multiple lines.
     */
   HYPER_ERROR_FIELD_HINT_MESSAGE = 3,
   /** The severity of the given error object. Always present. */
   HYPER_ERROR_FIELD_SEVERITY = 4,
   /**
     * The SQLSTATE code for the error.
     *
     * The SQLSTATE code identifies the type of error that has occurred; it can be used by front-end applications to
     * perform specific operations (such as error handling) in response to a particular database error.
     *
     * If the error category is HYPER_ERROR_CATEGORY_SQLSTATE, the value is the 5-character representation of
     * the SQLSTATE error code. Otherwise, the value is an empty string.
     *
     * Always present.
     * Read-only.
     */
   HYPER_ERROR_FIELD_SQL_STATE = 5,
   /** The associated causing error of the given error object, if any. */
   HYPER_ERROR_FIELD_CAUSE = 6,
   /** The context id. A unique identifier of the throw statement from which this error originated. */
   HYPER_ERROR_FIELD_CONTEXT_ID = 7,
   /** The detail part of the message. */
   HYPER_ERROR_FIELD_DETAIL = 8
} hyper_error_field_key;

/**
 * Constructs a new error object.
 *
 * The created error object will initially represent error category `HYPER_ERROR_CATEGORY_SQLSTATE` and error code 0.
 *
 * \param contextId  The context id, i.e., an unique identifier of the throw statement that created this error.
 * \return A pointer to the created error object, or NULL if there was not enough memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_error_create(uint32_t contextId) HYPER_API_NOEXCEPT;

/**
 * Destroys the given error object.
 *
 * \param error The error object to destroy.
 * \pre `error` is a valid, non-NULL pointer.
 */
HYPER_API void hyper_error_destroy(hyper_error_t* error) HYPER_API_NOEXCEPT;

/**
 * Represents the field value of an error object.
 */
struct hyper_error_field_value {
   int discriminator;
   union {
      int integer;
      const char* string;
      void* pointer;
      uint32_t uinteger;
   } value;
};

/**
 * Returns the value of the specified field of the given error object, if available.
 *
 * String values are returned as a pointer to a null-terminated UTF-8-encoded string.
 *
 * \param error  The error object.
 * \param key  The key of the field to get.
 * \param [out] value  The `hyper_error_field_value` object to store the value in.
 * \pre `error` is a valid, non-NULL pointer.
 * \pre `value` is a valid, non-NULL pointer.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_error_get_field(const hyper_error_t* error, hyper_error_field_key key, struct hyper_error_field_value* value)
   HYPER_API_NOEXCEPT;

/**
 * Sets the value of the specified field of the given error object.
 *
 * If the field is read-only, returns an error.
 *
 * \param error  The error object.
 * \param field  The key of the field to set.
 * \param value  The value of the field to set.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_error_set_field(hyper_error_t* error, hyper_error_field_key field, struct hyper_error_field_value value)
   HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Parameters
 *
 * These functions operate on parameter objects, which are collections of key/value pairs.
 *
 * \see hyper_create_instance_parameters
 * \see hyper_create_connection_parameters
 *
 * @{
 */

/**
 * An object representing a collection of key/value parameters.
 */
typedef struct hyper_parameters_t hyper_parameters_t;

/**
 * Destroy a connection parameters object.
 *
 * Also frees memory used by the `hyper_parameters_t` object.
 *
 * \param parameters  The parameters.
 * \pre `parameters` is a valid, non-NULL pointer.
 */
HYPER_API void hyper_parameters_destroy(hyper_parameters_t* parameters) HYPER_API_NOEXCEPT;

/**
 * Copies the given parameters object.
 *
 * \param parameters  The parameters object to copy.
 * \pre `parameters` is a valid, non-NULL pointer.
 * \return A pointer to the created copy of `parameters`, or NULL if not enough memory was available.
 */
HYPER_API hyper_parameters_t* hyper_parameters_copy(const hyper_parameters_t* parameters) HYPER_API_NOEXCEPT;

/**
 * Adds a parameter.
 *
 * The currently recognized connection parameter keys are:
 *
 * TODO Add list of valid connection parameter keys supported by Hyper (TFS 874568)
 * TODO Factor this list into a separate Doxygen section or page.
 *
 * \param parameters  The parameters object.
 * \param key  The key of the parameter (encoded as a UTF-8 string).
 * \param value  The value of the parameter (encoded as a UTF-8 string).
 * \pre `parameters` is a valid, non-NULL pointer.
 * \return Whether adding the parameter was successful.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_parameters_set(hyper_parameters_t* parameters, const char* key, const char* value) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Controlling an instance of the Hyper server
 *
 * These functions deal with starting and stopping Hyper instances. Before connecting to a Hyper instance,
 * `hyper_instance_error` should be called to check for errors.
 *
 * @{
 */

/**
 * A `hyper_instance_t` object represents a running Hyper instance.
 */
typedef struct hyper_instance_t hyper_instance_t;

/**
 * Hyper Telemetry.
 *
 */
typedef enum {
   /** Disable sharing usage data with Tableau */
   HYPER_DISABLE_TELEMETRY = 0,
   /** Enable sharing usage data with Tableau */
   HYPER_ENABLE_TELEMETRY = 1
} hyper_telemetry_t;

/**
 * Create an instance parameters object.
 *
 * The created `hyper_parameters_t` object must be destroyed via `hyper_parameters_destroy` to
 * free any associated memory.
 *
 * \param [out] parameters  A pointer to the instance parameters object, or NULL if not enough memory was available.
 * \param use_defaults  Whether the instance parameters should be initialized with default values.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_create_instance_parameters(hyper_parameters_t** parameters, bool use_defaults) HYPER_API_NOEXCEPT;

/**
 * Starts a Hyper server instance.
 *
 * The provided path is used to locate the Hyper executable. If \p hyper_path is NULL, the Hyper executable is looked
 * up in certain known locations.
 *
 * The returned `hyper_instance_t` object must be stopped via `hyper_instance_shutdown()` or `hyper_instance_close()`
 * to free any associated memory.
 *
 * \param hyper_path  The file path to the Hyper executable.
 * \param telemetry  Whether usage data should be sent to Tableau.
 * \param parameters  The instance parameters.
 * \param [out] instance  The Hyper instance or NULL if not enough memory was available.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_instance_create(
   const char* hyper_path, hyper_telemetry_t telemetry, const hyper_parameters_t* parameters, hyper_instance_t** instance)
   HYPER_API_NOEXCEPT;

/**
 * Stops the given Hyper server instance within `timeoutMs` milliseconds and frees the associated memory.
 *
 * Hyper is killed forcefully when the timeout is reached. In this case an error is returned. A timeout of -1
 * means waiting indefinitely for Hyper to shut down gracefully.
 *
 * An error is returned if shutdown is called after the hyper process exited with a non-zero exit code.
 *
 * \param instance  The Hyper instance.
 * \param timeoutMs The timeout in milliseconds after which Hyper is killed forcefully; -1 for no timeout.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 * \pre `instance` is a valid, non-NULL pointer.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_instance_shutdown(hyper_instance_t* instance, int timeoutMs) HYPER_API_NOEXCEPT;

/**
 * Stops the given Hyper server instance and frees the associated memory.
 *
 * Hyper is killed forcefully when the default timeout of 5 seconds is reached.
 *
 * Note that even if the attempt to start Hyper fails (as indicated by `hyper_instance_error`), the
 * application should call `hyper_instance_close()` to free the memory used by the `hyper_instance_t` object. The
 * `hyper_instance_t` pointer must not be used again after `hyper_instance_close()` has been called.
 *
 * \param instance  The Hyper instance.
 * \pre `instance` is a valid, non-NULL pointer.
 */
HYPER_API void hyper_instance_close(hyper_instance_t* instance) HYPER_API_NOEXCEPT;

/**
 * Returns the endpoint descriptor for the given Hyper instance.
 *
 * \param instance  The Hyper instance.
 * \return
 *    If the Hyper instance is not running, returns "".
 *    Otherwise, returns the endpoint descriptor as a UTF-8-encoded C string.
 *    The string is guaranteed to remain valid until a subsequent Hyper API call.// <-- XXX phrase better
 * \pre `instance` is a valid, non-NULL pointer.
 */
HYPER_API const char* hyper_instance_get_endpoint_descriptor(const hyper_instance_t* instance) HYPER_API_NOEXCEPT;

/**
 * Returns whether the instance process has exited. If so, `exit_code` is set to the exit code of the process.
 * Otherwise `exit_code` remains unchanged.
 * Note: The returned exit code is invalid if the process was terminated.
 *
 * \param instance The instance.
 * \param exit_code [out]  The exit code.
 * \pre `instance` is a valid, non-NULL pointer.
 * \pre `exit_code` is a valid, non-NULL pointer.
 * \return Whether the instance process has exited.
 */
HYPER_API bool hyper_instance_get_exit_code(hyper_instance_t* instance, int* exit_code) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Connecting to a running Hyper instance
 *
 * These functions deal with making a connection to a Hyper server. An application program can have several
 * backend connections open at one time. Each connection is represented by a `hyper_connection_t` object, which is
 * obtained from the function `hyper_connect()` or `hyper_connect_instance()`. Note that these functions will always
 * return a non-null object pointer, unless there is too little memory even to allocate the `hyper_connection_t` object.
 * The `hyper_connection_status()` function should be called to check the return value for a successful connection
 * before queries are sent via the connection object.
 *
 * @{
 */

/**
 * An object representing a connection to a Hyper instance.
 */
typedef struct hyper_connection_t hyper_connection_t;

/**
 * Create a connection parameters object.
 *
 * The returned `hyper_parameters_t` object must be destroyed via `hyper_parameters_destroy` to
 * free any associated memory.
 *
 * \param instance  The Hyper instance to connect to, or NULL.
 * \param [out] parameters  Output parameter for the pointer to the new connection parameters object.
 *    Will be set to NULL if not enough memory was available.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 */
HYPER_API hyper_error_t* hyper_create_connection_parameters(const hyper_instance_t* instance, hyper_parameters_t** parameters) HYPER_API_NOEXCEPT;

/**
 * Database creation mode for use with `hyper_connect()`.
 */
typedef enum {
   /// Do not create the database. Method will fail if database doesn't exist.
   HYPER_DO_NOT_CREATE = 0,
   /// Create the database. Method will fail if the database already exists.
   HYPER_CREATE,
   /// Create the database if it doesn't exist.
   HYPER_CREATE_IF_NOT_EXISTS,
   /// Create the database. If it already exists, drop the old one first.
   HYPER_CREATE_AND_REPLACE
} hyper_create_mode_t;

/**
 * Open a new connection to a Hyper instance.
 *
 * This function opens a new database connection using the passed parameters.
 *
 * The passed parameters can be empty to use all default parameters, or can contain one or more parameter settings. If
 * any parameter is an empty string, the corresponding environment variable is checked. If the environment variable is
 * not set either, then the indicated built-in defaults are used. Parameters are processed in index order. The effect of
 * this is that when parameters are repeated, the last processed value is retained.
 *
 * The returned `hyper_connection_t` object must be closed via `hyper_disconnect` to free any associated memory.
 *
 * \param parameters  The connection parameters.
 * \param [out] connection  Receives the new collection handle on success.
 * \param mode  The database creation mode.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_connect(const hyper_parameters_t* parameters, hyper_connection_t** connection, hyper_create_mode_t mode)
   HYPER_API_NOEXCEPT;

/**
 * Closes the connection to the server.
 *
 * Also frees memory used by the `hyper_connection_t` object.
 *
 * Note that even if the server connection attempt fails (as indicated by `hyper_connection_status`), the application
 * should call `hyper_disconnect` to free the memory used by the `hyper_connection_t` object. The `hyper_connection_t`
 * pointer must not be used again after `hyper_disconnect` has been called.
 *
 * \param connection  The connection.
 */
HYPER_API void hyper_disconnect(hyper_connection_t* connection) HYPER_API_NOEXCEPT;

/**
 * Cancel the current SQL command of the given connection (if any).
 *
 * Note: This call is thread-safe.
 *
 * \param connection The connection of which the currently executed query should be canceled.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_cancel(hyper_connection_t* connection) HYPER_API_NOEXCEPT;

/**
 * Status of the database server, as returned by `hyper_ping()`.
 */
typedef enum {
   /// The server is running and appears to be accepting connections.
   HYPER_PING_OK,
   /// The server is running but is in a state that disallows connections (startup, shutdown, or crash recovery).
   HYPER_PING_REJECT,
   /// The server could not be contacted. This might indicate that the server is not running, or that there is
   /// something wrong with the given connection parameters (for example, wrong port number), or that there is a
   /// network connectivity problem (for example, a firewall blocking the connection request).
   HYPER_PING_NO_RESPONSE,
   /// No attempt was made to contact the server, because the supplied parameters were obviously incorrect or there was
   /// some client-side problem (for example, out of memory).
   HYPER_PING_NO_ATTEMPT
} hyper_ping_status_t;

/**
 * Report the status of the database server.
 *
 * `hyper_ping accepts connection parameters identical to those of `hyper_connect`, described above. It is not necessary
 * to supply correct user name, password, or database name values to obtain the server status. However, if incorrect
 * values are provided, the server will log a failed connection attempt.
 *
 * \param parameters  The parameters.
 * \param [out] ping_status  Receives the ping status on success.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_ping(const hyper_parameters_t* parameters, hyper_ping_status_t* ping_status) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Querying the Status of a Database Connection
 *
 * These functions can be used to interrogate the status of an existing database connection object.
 *
 * @{
 */

/**
 * The status of a connection.
 */
typedef enum {
   HYPER_CONNECTION_OK,
   HYPER_CONNECTION_BAD
} hyper_connection_status_t;

/**
 * Returns the status of the connection.
 *
 * The status can be either OK or BAD. A successful connection to the database has the status HYPER_CONNECTION_OK. A
 * failed connection attempt is signaled by status HYPER_CONNECTION_BAD. Ordinarily, an OK status will remain so until
 * `hyper_disconnect()`, but a communications failure might result in the status changing to HYPER_CONNECTION_BAD.
 *
 * \param connection The connection.
 * \return The status of the connection.
 */
HYPER_API hyper_connection_status_t hyper_connection_status(hyper_connection_t* connection) HYPER_API_NOEXCEPT;

/**
 * Checks whether the connection is ready, i.e., if the connection can be used. A connection that is not ready is currently processing
 * a query, so using it for further query methods will result in an error.
 * Note that this method is not thread-safe; only use it on the same thread that potentially uses the connection.
 * (Note that a non-ready connection doesn't mean that the thread is in a blocking call; an open Result for example always keeps the connection busy)
 *
 * \pre connection != nullptr
 * \param connection  The connection.
 * \return whether the connection is ready
 */
HYPER_API bool hyper_connection_is_ready(hyper_connection_t* connection) HYPER_API_NOEXCEPT;

/**
 * Looks up a current parameter setting of the server.
 *
 * Certain parameter values are reported by the server automatically at connection startup or whenever their values
 * change. `hyper_connection_parameter_status()` can be used to interrogate these settings.
 *
 * TODO: Validate that the following list is correct and complete for Hyper (TFS 874568)
 *
 * Parameters reported as of the current release include server_version, server_encoding, client_encoding,
 * application_name, is_superuser, session_authorization, DateStyle, IntervalStyle, TimeZone, integer_datetimes, and
 * standard_conforming_strings. Note that server_version, server_encoding and integer_datetimes cannot change after
 * startup.
 *
 * If no value for standard_conforming_strings is reported, applications can assume it is off, that is, backslashes are
 * treated as escapes in string literals. Also, the presence of this parameter can be taken as an indication that the
 * escape string syntax (E'...') is accepted.
 *
 * \param connection The connection.
 * \param parameter_name The parameter name (encoded as a UTF-8 string).
 * \return The current value of the parameter if known (encoded as a UTF-8 string), or NULL if the parameter is not
 * known. The value's life-time is bound to the connection.
 */
HYPER_API const char* hyper_connection_parameter_status(const hyper_connection_t* connection, const char* parameter_name) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Processing Notice Messages
 *
 * These functions deal with the processing of notice messages received from Hyper.
 *
 * @{
 */

/**
 * A callback for notices.
 *
 * When a notice or warning message is received from the server, the notice receiver function is called. It is passed
 * the message in the form of a `hyper_error_t`. This allows the receiver to extract individual fields using
 * `hyper_error_t` message and field accessors. The same void pointer passed to `hyper_set_notice_receiver`
 * is also passed (this pointer can be used to access application-specific state if needed). The caller must
 * eventually call `hyper_error_destroy()` to free any memory associated with every notice each time this
 * notice receiver is invoked.
 */
typedef void (*hyper_notice_receiver_t)(void* context, hyper_error_t* notice);

/**
 * Sets or examines the current notice receiver for a connection object
 *
 * Notice and warning messages generated by the server are not returned by the query execution functions, since they do
 * not imply failure of the query. Instead they are passed to a notice handling function, and execution continues
 * normally after the handler returns. The default notice handling function prints the message on stderr, but the
 * application can override this behavior by supplying its own handling function.
 *
 * `hyper_set_notice_receiver` sets a new notice receiver for a connection. If you supply a null function pointer, no
 * action is taken, but the current pointer is returned.
 *
 * Once you have set a notice receiver, you should expect that that function could be called as long as either the
 * `hyper_connection_t` object exists.
 *
 * \param connection The connection.
 * \param receiver The new notice receiver or NULL if the current notice receiver should not be changed.
 * \param context Some custom context.
 * \return The previous notice receiver.
 */
HYPER_API hyper_notice_receiver_t hyper_set_notice_receiver(hyper_connection_t* connection, hyper_notice_receiver_t receiver, void* context) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Data Definition
 *
 * These functions allow to manage schema objects such as databases, schemas and tables in Hyper without using SQL.
 *
 * @{
 */

/**
 * Create a database.
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param path  The path to the database to create (encoded as a UTF-8 string).
 * \param failIfExists  Whether to fail if the database already exists.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_create_database(hyper_connection_t* connection, const char* path, bool failIfExists) HYPER_API_NOEXCEPT;

/**
 * Drop a database.
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param path  The path to the database to drop (encoded as a UTF-8 string).
 * \param failIfNotExists  Whether to fail if the database does not exist.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_drop_database(hyper_connection_t* connection, const char* path, bool failIfNotExists) HYPER_API_NOEXCEPT;

/**
 * Detach all databases from the session.
 *
 * If this connection was the only connection that was currently attached to these databases,
 * then the database files will be unloaded, all updates will be written, and the database file can
 * be accessed externally (e.g., copied, moved, etc.).
 *
 * This call will block until all databases are detached. As this might trigger multiple unloads, this call
 * can block for a considerable amount of time, if many updates have to be written to disk.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_detach_all_databases(hyper_connection_t* connection) HYPER_API_NOEXCEPT;

/**
 * Detaches a database from the connection.
 *
 * If this connection was the only connection that was currently attached to the database,
 * then the database file will be unloaded, all updates will be written, and the database file can
 * be accessed externally (e.g., copied, moved, etc.).
 *
 * This call will block until the database is detached. As this might trigger a unload, this call
 * can block for a considerable amount of time, if many updates have to be written to disk.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param alias  The alias of the database.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_detach_database(hyper_connection_t* connection, const char* alias) HYPER_API_NOEXCEPT;

/**
 * Attaches a database to the connection.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param path  The path to the database.
 * \param alias  The alias under which the database will be attached. No explicit alias is used if a `nullptr` is passed here.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_attach_database(hyper_connection_t* connection, const char* path, const char* alias) HYPER_API_NOEXCEPT;

/**
 * Create a Schema.
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param databaseName  The name of the database in which the schema should be created (encoded as a UTF-8 string) or `nullptr` to deduce the database name from the schema search path.
 * \param schemaName  The name of the schema to create (encoded as a UTF-8 string).
 * \param failIfExists  Whether to fail if the schema already exists.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_create_schema(hyper_connection_t* connection, const char* databaseName, const char* schemaName, bool failIfExists) HYPER_API_NOEXCEPT;

/**
 * A table definition.
 *
 * Defines the data type of each column.
 */
typedef struct hyper_table_definition_t hyper_table_definition_t;

/**
 * Possible types of table columns in Hyper.
 */
typedef enum {
   HYPER_UNSUPPORTED = 0,
   HYPER_BOOL = 1,
   HYPER_BIG_INT = 2,
   HYPER_SMALL_INT = 3,
   HYPER_INT = 4,
   HYPER_NUMERIC = 5,
   HYPER_DOUBLE = 6,
   HYPER_OID = 7,
   HYPER_BYTE_A = 8,
   HYPER_TEXT = 9,
   HYPER_VARCHAR = 10,
   HYPER_CHAR = 11,
   HYPER_JSON = 12,
   HYPER_DATE = 13,
   HYPER_INTERVAL = 14,
   HYPER_TIME = 15,
   HYPER_TIMESTAMP = 16,
   HYPER_TIMESTAMP_TZ = 17,
   HYPER_GEOGRAPHY = 18,
} hyper_type_tag_t;

/**
 * A field index.
 */
typedef uint32_t hyper_field_index_t;

/**
 * A row index.
 */
typedef uint32_t hyper_row_index_t;

/**
 * A type modifier.
 */
typedef uint32_t hyper_type_modifier_t;

/**
 * Possible persistence levels of a table.
 */
typedef enum {
   HYPER_PERMANENT = 0,
   HYPER_TEMPORARY = 1
} hyper_table_persistence_t;

/**
 * Creates a table definition.
 *
 * The returned `hyper_table_definition_t` object must be destroyed via `hyper_destroy_table_definition()` to free any
 * associated memory.
 *
 * \param database_name The (optional) unescaped name of the database (encoded as a UTF-8 string).
 * \param schema_name The (optional) unescaped name of the schema (encoded as a UTF-8 string).
 * \param table_name The unescaped name of the table (encoded as a UTF-8 string).
 * \param persistence  The persistence of the table
 * \param stream  Whether the definition is for a stream
 * \return The table definition or NULL if not enough memory was available.
 */
HYPER_API hyper_table_definition_t* hyper_create_table_definition(
   const char* database_name, const char* schema_name, const char* table_name, hyper_table_persistence_t persistence, bool stream) HYPER_API_NOEXCEPT;

/**
 * Retrieves a table definition.
 *
 * The returned `hyper_table_definition_t` object must be destroyed via `hyper_destroy_table_definition()` to free any
 * associated memory.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param database_name  The (optional) unescaped name of the database (encoded as a UTF-8 string). If not mentioned, the database name will be deduced from the
 *    set search path or schema search path.
 * \param schema_name The (optional) unescaped name of the schema (encoded as a UTF-8 string). If NULL, then the result
 *    table definition will be deduced from the set search path.
 * \param table_name  The unescaped name of the table (encoded as a UTF-8 string).
 * \param [out] table_definition  The table definition.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error. The caller _must_ handle
 *    this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API hyper_error_t* hyper_get_table_definition(
   hyper_connection_t* connection, const char* database_name, const char* schema_name, const char* table_name, hyper_table_definition_t** table_definition)
   HYPER_API_NOEXCEPT;

/**
 * Destroys the given table definition.
 *
 * Also frees memory used by the `hyper_table_definition_t` object.
 *
 * \param table_definition The table definition to destroy.
 */
HYPER_API void hyper_destroy_table_definition(hyper_table_definition_t* table_definition) HYPER_API_NOEXCEPT;

/**
 * Creates a copy of the given table definition.
 *
 * \param table_definition The table definition to copy.
 * \param [out] copy Receives the copy on success.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API hyper_error_t* hyper_copy_table_definition(const hyper_table_definition_t* table_definition, hyper_table_definition_t** copy) HYPER_API_NOEXCEPT;

/**
 * Returns a database name.
 *
 * \param table_definition The table definition.
 * \return The name of the database (encoded as a UTF-8 string).
 */
HYPER_API const char* hyper_table_definition_database_name(const hyper_table_definition_t* table_definition) HYPER_API_NOEXCEPT;

/**
 * Returns a schema name.
 *
 * \param table_definition The table definition.
 * \return The name of the schema (encoded as a UTF-8 string).
 */
HYPER_API const char* hyper_table_definition_schema_name(const hyper_table_definition_t* table_definition) HYPER_API_NOEXCEPT;

/**
 * Returns a table name.
 *
 * \param table_definition The table definition.
 * \return The name of the table (encoded as a UTF-8 string).
 */
HYPER_API const char* hyper_table_definition_table_name(const hyper_table_definition_t* table_definition) HYPER_API_NOEXCEPT;

/**
 * Returns the type of the table.
 *
 * \param table_definition The table definition.
 * \return The type of the table.
 */
HYPER_API hyper_table_persistence_t hyper_table_definition_table_persistence(const hyper_table_definition_t* table_definition) HYPER_API_NOEXCEPT;

/**
 * Returns the number of columns in the given table definition.
 *
 * \param table_definition  The table definition.
 * \return The number of columns.
 */
HYPER_API size_t hyper_table_definition_column_count(const hyper_table_definition_t* table_definition) HYPER_API_NOEXCEPT;

/**
 * Returns the type tag of the column associated with the given index in the given table definition.
 *
 * \param table_definition  The table definition.
 * \param column_index  The column index. Column indexes start at 0.
 * \return The type tag of the data type.
 */
HYPER_API hyper_type_tag_t hyper_table_definition_column_type_tag(const hyper_table_definition_t* table_definition, hyper_field_index_t column_index)
   HYPER_API_NOEXCEPT;

/**
 * Returns the oid of data type of the column associated with the given index in the given table definition.
 *
 * \param table_definition  The table definition.
 * \param column_index  The column index. Column indexes start at 0.
 * \return The oid of the data type.
 */
HYPER_API uint32_t hyper_table_definition_column_type_oid(const hyper_table_definition_t* table_definition, hyper_field_index_t column_index)
   HYPER_API_NOEXCEPT;

/**
 * Returns the type modifier of the column associated with the given index in the given table definition.
 *
 * The interpretation of modifier values is type-specific; they typically indicate precision or size limits. The value
 * -1 is used to indicate "no information available". Most data types do not use modifiers, in which case the value is
 * always -1.
 *
 * \param table_definition  The table definition.
 * \param column_index  The column index. Column indexes start at 0.
 * \return The type modifier.
 */
HYPER_API hyper_type_modifier_t hyper_table_definition_column_type_modifier(const hyper_table_definition_t* table_definition, hyper_field_index_t column_index)
   HYPER_API_NOEXCEPT;

/**
 * Returns the max length if it is encoded in the given type modifier.
 *
 * The interpretation of modifier values is type-specific; they typically indicate precision or size limits. The value
 * 0 is used to indicate "no information available". Most data types do not use modifiers, in which case the value is
 * always 0.
 *
 * \param modifier A type modifier for a type that encodes the max length (e.g., text types).
 * \return The max length if it is encoded in the given type modifier.
 */
HYPER_API uint32_t hyper_get_max_length_from_modifier(hyper_type_modifier_t modifier) HYPER_API_NOEXCEPT;

/**
 * Returns the precision if it is encoded in the given type modifier.
 *
 * The interpretation of modifier values is type-specific; they typically indicate precision or size limits. The value
 * 0 is used to indicate "no information available". Most data types do not use modifiers, in which case the value is
 * always 0.
 *
 * \param modifier A type modifier for a type that encodes the precision (e.g., numerics).
 * \return The precision if it is encoded in the given type modifier.
 */
HYPER_API uint32_t hyper_get_precision_from_modifier(hyper_type_modifier_t modifier) HYPER_API_NOEXCEPT;

/**
 * Returns the scale if it is encoded in the given type modifier.
 *
 * The interpretation of modifier values is type-specific; they typically indicate precision or size limits. The value
 * 0 is used to indicate "no information available". Most data types do not use modifiers, in which case the value is
 * always 0.
 *
 * \param modifier A type modifier for a type that encodes the scale (e.g., numeric types).
 * \return The scale if it is encoded in the given type modifier.
 */
HYPER_API uint32_t hyper_get_scale_from_modifier(hyper_type_modifier_t modifier) HYPER_API_NOEXCEPT;

/**
 * Encodes the type modifier parts of a numeric into a 32bit value.
 *
 * @param precision The precision of the numeric.
 * @param scale The scale of the numeric.
 * @return The 32bit encoding of the parameters.
 */
HYPER_API hyper_type_modifier_t hyper_encode_numeric_modifier(uint32_t precision, uint32_t scale) HYPER_API_NOEXCEPT;

/**
 * Encodes the type modifier parts of a string type into a 32bit value.
 *
 * @param max_length The max length of the string type.
 * @return The 32bit encoding of the parameters.
 */

HYPER_API hyper_type_modifier_t hyper_encode_string_modifier(uint32_t max_length) HYPER_API_NOEXCEPT;

/**
 * Returns the column index associated with the given column name in the given table definition.
 *
 * \param table_definition  The table definition.
 * \param column_name  The column name (encoded as a UTF-8 string).
 * \return The column index corresponding to the column name.
 */
HYPER_API hyper_field_index_t hyper_table_definition_column_index(const hyper_table_definition_t* table_definition, const char* column_name) HYPER_API_NOEXCEPT;

/**
 * Returns the name of the column with the given index in the given table definition.
 *
 * Field indexes start at 0.
 *
 * \param table_definition  The table definition.
 * \param column_index  The column index.
 * \return The column name (encoded as a UTF-8 string).
 */
HYPER_API const char* hyper_table_definition_column_name(const hyper_table_definition_t* table_definition, hyper_field_index_t column_index) HYPER_API_NOEXCEPT;

/**
 * Returns true if the column with the given index in the given table definition is nullable.
 *
 * Field indexes start at 0.
 *
 * \param table_definition  The table definition.
 * \param column_index  The column index.
 * \return True if the column is nullable.
 */
HYPER_API bool hyper_table_definition_column_is_nullable(const hyper_table_definition_t* table_definition, hyper_field_index_t column_index) HYPER_API_NOEXCEPT;

/**
 * Returns the collation of the column with the given index in the given table definition.
 *
 * Field indexes start at 0
 *
 * \param table_definition  The table definition.
 * \param column_index  The column index.
 * \return The column collation.
 */
HYPER_API const char* hyper_table_definition_column_collation(const hyper_table_definition_t* table_definition, hyper_field_index_t column_index)
   HYPER_API_NOEXCEPT;

// TODO: Consider specifying column types for `hyper_add_column()` as string, e.g., "varchar(255)" (TFS 874040)

/**
 * A marker for an unspecified type modifier.
 *
 * \see hyper_add_column()
 */
#define HYPER_UNUSED_MODIFIER ~0u

/**
 * Add a column to a table definition.
 *
 * The following column types can be adapted by providing a primary and/or a secondary length:
 * - The string types HYPER_TEXT and HYPER_CHARACTER support a primary length that determines the maximum number of
 *   characters stored for each string in the column. A column of type HYPER_CHARACTER without a primary length stores
 *   one character by default. A column of type HYPER_TEXT with no primary length can store strings with up to 2 GB.
 * - The type HYPER_NUMERIC supports a primary and a secondary length. The primary length determines the precision and
 *   the secondary the scale of the numeric values stored. The precision is the total number of digits, while the scale
 *   is the number of decimal places. For example, the number 123.4567 has a precision of seven and a scale of four. A
 *   numeric without secondary length has scale 0.
 *
 * \param table_definition The table definition to which the column should be added.
 * \param column_name The name of the new column (encoded as a UTF-8 string).
 * \param type_tag The type of the new column.
 * \param modifier The optional type modifier that encodes, e.g., the length for text types or precision and scale for numerics.
 * \param collation The collation encoded as a UTF-8 string (valid for HYPER_TEXT, HYPER_CHARACTER, and HYPER_CLOB).
 * \param nullable Whether the column should be able to store null values.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_table_definition_add_column(
   hyper_table_definition_t* table_definition,
   const char* column_name,
   hyper_type_tag_t type_tag,
   hyper_type_modifier_t modifier,
   const char* collation,
   bool nullable) HYPER_API_NOEXCEPT;

/**
 * Create a table.
 *
 * If `failIfExists` is true, then the method will return an error if a table with this name  already exists.
 * Otherwise, it will just silently do nothing in this case.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection The connection.
 * \param table_definition The table definition describing the columns of the new table.
 * \param failIfExists  Whether this method should return an error if a table with the given name already exists.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_create_table(
   hyper_connection_t* connection, const hyper_table_definition_t* table_definition, bool failIfExists) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Executing SQL Queries
 *
 * These functions allow to efficiently execute SQL queries against Hyper.
 *
 * Usage example (query, chunk mode):
 *
 *     auto rowset = hyper_execute_query(conn, query);
 *     auto metadata = hyper_rowset_get_table_definition(rowset);
 *     size_t columnCount = hyper_table_definition_column_count(metadata);
 *     std::vector<std::future<void>> chunk_processors;
 *
 *     while (true)
 *     {
 *         hyper_rowset_chunk_t* chunk = NULL;
 *         hyper_error_t* error = hyper_rowset_get_next_chunk(rowset, &chunk);
 *         if (error)
 *         {
 *             // ... do some error processing with the given error.
 *             hyper_error_destroy(error);
 *             break;
 *         }
 *
 *         if (!chunk)
 *         {
 *             // The end of the rowset was reached.
 *             break;
 *         }
 *
 *         chunk_processors.emplace_back(std::async(std::launch::async, [chunk]
 *         {
 *             size_t rowCount = hyper_rowset_chunk_row_count(chunk);
 *             for (int row_index = 0; row_index < row_count; ++row_index)
 *             {
 *                 for (hyper_field_index_t field_index = 0; field_index < columnCount; ++field_index)
 *                 {
 *                     if (hyper_rowset_field_is_null(chunk, row_index, field_index))
 *                     {
 *                         // ... handle NULL value.
 *                         continue;
 *                     }
 *
 *                     auto field_value = hyper_rowset_field_value(chunk, row_index, field_index);
 *                     // ... process field_value.value and field_value.size.
 *                 }
 *             }
 *             hyper_destroy_rowset_chunk(chunk);
 *         }));
 *     }
 *
 *     for (auto& chunk_processor : chunk_processors)
 *         chunk_processor.get();
 *
 *     hyper_close_rowset(rowset);
 *
 *
 * @{
 */

/**
 * The result of a query.
 */
typedef struct hyper_rowset_t hyper_rowset_t;

/**
 * A rowset chunk.
 */
typedef struct hyper_rowset_chunk_t hyper_rowset_chunk_t;

/**
 * A value of a field.
 */
typedef struct
{
   const uint8_t* value;
   size_t size;
} hyper_value_t;

/**
 * Rowset result format.
 */
typedef enum {
   HYPER_ROWSET_RESULT_FORMAT_TEXT = 0,
   HYPER_ROWSET_RESULT_FORMAT_HYPER_BINARY = 2
} hyper_rowset_result_format_t;

/**
 * Enable or disable retrieval of query result rowset in a chunked mode.
 *
 * \param connection  The connection.
 * \param chunked_mode Whether to enable chunked mode.
 */
HYPER_API void hyper_set_chunked_mode(hyper_connection_t* connection, bool chunked_mode) HYPER_API_NOEXCEPT;

/**
 * Sets the query result rowset prefetch threshold in bytes.
 *
 * The rowset will spawn an async thread to fetch chunks until first exceeding this threshold. Disable the async thread by setting to 0.
 *
 * \param connection  The connection.
 * \param prefetch_threshold The new prefetch threshold.
 */
HYPER_API void hyper_set_prefetch_threshold(hyper_connection_t* connection, size_t prefetch_threshold) HYPER_API_NOEXCEPT;

/**
 * Execute a SQL query.
 *
 * The returned `hyper_rowset_t` object must be closed via `hyper_close_rowset()` to free any associated memory. While the rowset remains open,
 * the connection will be busy (= not ready).
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param query  The SQL query (encoded as a UTF-8 string).
 * \param [out] rowset  Receives the new rowset handle on success.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
// TODO: Reconsider result processing in rowset API to provide size of rowset (TFS 874037)
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_execute_query(hyper_connection_t* connection, const char* query, hyper_rowset_t** rowset) HYPER_API_NOEXCEPT;

/**
 * Execute a SQL command.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param query  The SQL command (encoded as a UTF-8 string).
 * \param [out] affected_row_count  Receives the number of affected rows on success if available; -1 otherwise.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_execute_command(hyper_connection_t* connection, const char* query, int* affected_row_count)
   HYPER_API_NOEXCEPT;

/**
 * Execute a SQL query with advanced execution options.
 *
 * The returned `hyper_rowset_t` object must be closed via `hyper_close_rowset()` to free any associated memory. While the rowset remains open,
 * the connection will be busy (= not ready).
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param query  The SQL query (encoded as a UTF-8 string).
 * \param result_format  Rowset result format.
 * \param [out] rowset  Receives the new rowset handle on success.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
// TODO: Reconsider result processing in rowset API to provide size of rowset (TFS 874037)
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_execute_query_params(
   hyper_connection_t* connection, const char* query, hyper_rowset_result_format_t result_format, hyper_rowset_t** rowset) HYPER_API_NOEXCEPT;

/**
 * An object representing a collection of strings.
 */
typedef struct hyper_string_list_t hyper_string_list_t;

/**
 * Destroys a string list object.
 *
 * Also frees memory used by the `hyper_string_list_t` object.
 *
 * \param string_list  The string list.
 * \pre `string_list` is a valid string list pointer or a nullptr
 */
HYPER_API void hyper_string_list_destroy(hyper_string_list_t* string_list) HYPER_API_NOEXCEPT;

/**
 * Returns the size of the string list.
 * \param string_list  The string list.
 * \return The size of the list.
 */
HYPER_API size_t hyper_string_list_size(hyper_string_list_t* string_list) HYPER_API_NOEXCEPT;

/**
 * Returns the string at the given index in the string list.
 * \param string_list  The string list.
 * \param index  The index.
 * \return The stored value.
 * \pre index < hyper_string_list_size(string_list)
 */
HYPER_API const char* hyper_string_list_at(hyper_string_list_t* string_list, int index) HYPER_API_NOEXCEPT;

/**
 * Retrieves the names of all schemas in the given database.
 * The returned `hyper_string_list_t` object must be closed via `hyper_string_list_destroy()` to free any associated memory.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param database  The name of the database or `nullptr` to use the first database in the database search path.
 * \param [out] schema_names  Receives a handle for a string list containing the schema names on success.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *         The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_get_schema_names(hyper_connection_t* connection, const char* database, hyper_string_list_t** schema_names)
   HYPER_API_NOEXCEPT;

/**
 * Retrieves the names of all tables in the given schema.
 * The returned `hyper_string_list_t` object must be closed via `hyper_string_list_destroy()` to free any associated memory.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param database  The name of the database or `nullptr` to use the first database in the database search path.
 * \param schema The name of the schema.
 * \param [out] table_names  Receives a handle for a string list containing the table names on success.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *         The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_get_table_names(
   hyper_connection_t* connection, const char* database, const char* schema, hyper_string_list_t** table_names) HYPER_API_NOEXCEPT;

/**
 * Checks whether a table with the given (qualified) name exists.
 * If no explicit schema is given, this method will check if a table with the given name exists in the search path.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param database  The database, expects an unescaped name or nullptr.
 * \param schema  The schema, expects an unescaped name or nullptr.
 * \param table  The table, expects an unescaped name.
 * \param [out] exists  whether the table exists.
 * \return Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *         The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 * \pre If specified, the database name has to refer to an existing, attached database.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_has_table(
   hyper_connection_t* connection, const char* database, const char* schema, const char* table, bool* exists) HYPER_API_NOEXCEPT;

/**
 * Escape the given string for safe usage in SQL query or command strings as identifier.
 *
 * \param[out] target The target location to which the escaped string should be written. Allowed to be `nullptr` if `space` is 0.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to escape.
 * \param length The length of the value.
 * \return The number of bytes that have been written or would have been written if there was enough space. Returns 0 if not enough memory for internal
 * allocations was available on the machine.
 */
HYPER_API size_t hyper_quote_sql_identifier(char* target, size_t space, const char* value, size_t length) HYPER_API_NOEXCEPT;

/**
 * Escape the given string for safe usage in SQL query or command strings as string literal.
 *
 * \param[out] target The target location to which the escaped string should be written. Allowed to be `nullptr` if `space` is 0.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to escape.
 * \param length The length of the value.
 * \return The number of bytes that have been written or would have been written if there was enough space. Returns 0 if not enough memory for internal
 * allocations was available on the machine.
 */
HYPER_API size_t hyper_quote_sql_literal(char* target, size_t space, const char* value, size_t length) HYPER_API_NOEXCEPT;

/**
 * Prepare a SQL query.
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param statement_name  The name of the prepared statement (encoded as a UTF-8 string).
 * \param query  The SQL query (encoded as a UTF-8 string).
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API hyper_error_t* hyper_prepare(hyper_connection_t* connection, const char* statement_name, const char* query) HYPER_API_NOEXCEPT;

/**
 * Execute a prepared SQL query.
 *
 * The returned `hyper_rowset_t` object must be closed via `hyper_close_rowset()` to free any associated memory.  While the rowset remains open,
 * the connection will be busy (= not ready).
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection  The connection.
 * \param statement_name  The prepared statement name (encoded as a UTF-8 string).
 * \param result_format  The result format of the output of the prepared statement.
 * \param [out] rowset  Receives the new rowset handle on success.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API hyper_error_t* hyper_execute_prepared(
   hyper_connection_t* connection, const char* statement_name, hyper_rowset_result_format_t result_format, hyper_rowset_t** rowset) HYPER_API_NOEXCEPT;

/**
 * Close the given rowset. The associated connection will be ready afterwards.
 *
 * Also frees memory used by the `hyper_rowset_t` object.
 *
 * \param rowset The row set to close.
 */
HYPER_API void hyper_close_rowset(hyper_rowset_t* rowset) HYPER_API_NOEXCEPT;

/**
 * Returns the table definition of the given rowset.
 * Note that types of a rowset do not have nullability or collation information.
 *
 * TODO: The fields may change between calls to `hyper_rowset_next_row()` when the end of one result is reached and
 *       the next query of a multi-query SQL command is executed (TFS 874037).
 *
 * \param rowset  The rowset.
 * \return The table definition.
 */
HYPER_API const hyper_table_definition_t* hyper_rowset_get_table_definition(const hyper_rowset_t* rowset) HYPER_API_NOEXCEPT;

/**
 * Returns the affected row count of the given rowset.
 *
 * TODO: The fields may change between calls to `hyper_rowset_next_row()` when the end of one result is reached and
 *       the next query of a multi-query SQL command is executed (TFS 874037).
 *
 * \param rowset  The rowset.
 * \return The affected row count if available; -1 otherwise.
 */
HYPER_API int64_t hyper_rowset_get_affected_row_count(const hyper_rowset_t* rowset) HYPER_API_NOEXCEPT;

/**
 * Return the next chunk of the result or NULL if the end of the rowset was reached.
 *
 * TODO: When the SQL command consists of multiple queries, fetching the next row will at some point execute the next
 *       query. This invalidates field names, indexes, type modifiers, types, values, and the field count (TFS 874037).
 *
 * TODO: Generalize the `status` output parameter by introducing an enum.
 *
 * \param rowset  The row set.
 * \param [out] rowset_chunk  The next rowset chunk.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 *    The caller _must_ call hyper_destroy_rowset_chunk to free any memory associated with this rowset chunk.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_rowset_get_next_chunk(hyper_rowset_t* rowset, hyper_rowset_chunk_t** rowset_chunk) HYPER_API_NOEXCEPT;

/**
 * Returns the number of rows in the chunk.
 *
 * \param rowset_chunk  The rowset chunk.
 * \return The number of rows in the chunk.
 */
HYPER_API size_t hyper_rowset_chunk_row_count(const hyper_rowset_chunk_t* rowset_chunk) HYPER_API_NOEXCEPT;

/**
 * Returns the value of a rowset chunk field.
 *
 * \param rowset_chunk  The rowset chunk.
 * \param row_index  The row index.
 * \param field_index  The field index.
 * \return The value of the field.
 */
HYPER_API hyper_value_t hyper_rowset_chunk_field_value(const hyper_rowset_chunk_t* rowset_chunk, hyper_row_index_t row_index, hyper_field_index_t field_index)
   HYPER_API_NOEXCEPT;

/**
 * Returns the value of a rowset chunk field.
 * This variant is (currently) used by Python.
 *
 * \param rowset_chunk  The rowset chunk.
 * \param row_index  The row index.
 * \param field_index  The field index.
 * \param [out] sizeOut  Output parameter for the size of the value.
 * \return A pointer to the raw value.
 */
HYPER_API const uint8_t* hyper_rowset_chunk_field_value_byref(
   const hyper_rowset_chunk_t* rowset_chunk, hyper_row_index_t row_index, hyper_field_index_t field_index, int* sizeOut) HYPER_API_NOEXCEPT;

/**
 * Check whether the value of the rowset chunk field is null.
 *
 * \param rowset_chunk The rowset chunk.
 * \param row_index  The row index.
 * \param field_index  The field index.
 * \return Whether the value of the field is null.
 */
HYPER_API bool hyper_rowset_chunk_field_is_null(const hyper_rowset_chunk_t* rowset_chunk, hyper_field_index_t row_index, hyper_field_index_t field_index)
   HYPER_API_NOEXCEPT;

/**
 * Returns the values of all rowset chunk fields together with their sizes and null flags.
 *
 * \param rowset_chunk  The rowset chunk.
 * \param [out] col_count  Output parameter for the number of columns
 * \param [out] row_count  Output parameter for the number of rows
 * \param [out] values  Output parameter for the field values.
 * \param [out] sizes  Output parameter for the field value sizes.
 * \param [out] null_flags  Output parameter for the null flags.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_rowset_chunk_field_values(
   hyper_rowset_chunk_t* rowset_chunk, size_t* col_count, size_t* row_count, const uint8_t* const* values[], const size_t* sizes[], const int8_t* null_flags[])
   HYPER_API_NOEXCEPT;

/**
 * Destroy rowset chunk.
 *
 * \param rowset_chunk The rowset chunk.
 */
HYPER_API void hyper_destroy_rowset_chunk(const hyper_rowset_chunk_t* rowset_chunk) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * A Hyper binary data chunk.
 */
typedef struct hyper_data_chunk_t hyper_data_chunk_t;

/**
 * Create a data chunk of a default size (16MB including the Hyper binary header).
 *
 * The returned `hyper_data_chunk_t` object must be destroyed via `hyper_destroy_data_chunk()` to free any associated
 * memory.
 *
 * \return The data chunk or NULL if not enough memory was available.
 */
HYPER_API hyper_data_chunk_t* hyper_create_data_chunk(void) HYPER_API_NOEXCEPT;

/**
 * Resize the data chunk to the specified size.
 *
 * The returned `hyper_data_chunk_t` object must be destroyed via `hyper_destroy_data_chunk()` to free any associated
 * memory.
 *
 * \param data_chunk  The data chunk to resize.
 * \param size  The chunk size in bytes.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_resize_data_chunk(hyper_data_chunk_t* data_chunk, size_t size) HYPER_API_NOEXCEPT;

/**
 * Returns a pointer to the data chunk beginning.
 *
 * \param data_chunk The data chunk.
 * \return Pointer to the data chunk beginning.
 */
HYPER_API uint8_t* hyper_get_chunk_data(const hyper_data_chunk_t* data_chunk) HYPER_API_NOEXCEPT;

/**
 * Returns the size of the Hyper binary header.
 *
 * \param data_chunk The data chunk.
 * \return The size of the Hyper binary header.
 */
HYPER_API size_t hyper_get_chunk_header_size(const hyper_data_chunk_t* data_chunk) HYPER_API_NOEXCEPT;

/**
 * Returns the size of the data chunk.
 *
 * \param data_chunk The data chunk.
 * \return The size of the data chunk.
 */
HYPER_API size_t hyper_get_chunk_data_size(const hyper_data_chunk_t* data_chunk) HYPER_API_NOEXCEPT;

/**
 * Destroy a specified data chunk.
 *
 * \param data_chunk The data chunk to destroy.
 */
HYPER_API void hyper_destroy_data_chunk(hyper_data_chunk_t* data_chunk) HYPER_API_NOEXCEPT;

/**
 * \name Inserting Data
 *
 * These functions allow efficient data insertion into Hyper tables.
 *
 * Usage example:
 *
 *      hyper_inserter_t inserter = NULL;
 *      hyper_error_t* error = hyper_create_inserter(conn, table_definition, &inserter);
 *      if (error)
 *      {
 *          // ... do some error processing with the given error message string.
 *      };
 *
 *      // optionally enable bulk insertion
 *      error = hyper_init_bulk_insert(inserter, table_definition, select_list);
 *      if (error)
 *          handle_error();
 *
 *      // insert data chunks
 *      while (...)
 *      {
 *          error = hyper_inserter_insert_chunk(inserter, data_chunk, bytes);
 *          if (error)
 *              handle_error();
 *      }
 *
 *      error = hyper_close_inserter(inserter);
 *      if (error)
 *          handle_error();
 * @{
 */

/**
 * An inserter that allows to insert rows into a table.
 */
typedef struct hyper_inserter_t hyper_inserter_t;

/**
 * Create an inserter.
 *
 * The returned `hyper_inserter_t` object must be closed via `hyper_close_inserter()` to free any associated
 * memory. As long as the inserter remains open, the associated connection will be busy (= not ready).
 *
 * Returns an error if the connection is not ready.
 *
 * \param connection The connection.
 * \param table_definition  The definition of the table into which to insert new rows.
 * \param [out] inserter  Receives the new inserter object on success.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_create_inserter(
   hyper_connection_t* connection, const hyper_table_definition_t* table_definition, hyper_inserter_t** inserter) HYPER_API_NOEXCEPT;

/**
 * Initialize bulk inserter.
 *
 * \param inserter The inserter.
 * \param table_definition The definition of the stream into which data is inserted.
 * \param select_list Comma separated list of column names and expressions (encoded as UTF-8 string).
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_init_bulk_insert(hyper_inserter_t* inserter, const hyper_table_definition_t* table_definition, const char* select_list) HYPER_API_NOEXCEPT;

/**
 * Insert data generated by expressions.
 *
 * Generates an INSERT statement of the form `INSERT INTO table (column_list) (SELECT select_list)`
 * Usage example:
 *      // Inserts result of generate_series into target table
 *
 *      hyper_inserter_t inserter = NULL;
 *      hyper_error_t* error = hyper_create_inserter(conn, table_definition, &inserter);
 *      if (error)
 *      {
 *          // ... do some error processing with the given error message string.
 *      };
 *
 *      error = hyper_insert_computed_expressions(inserter, "generate_series(1,100)");
 *      if (error)
 *          handle_error();
 *
 *      error = hyper_close_inserter(inserter);
 *      if (error)
 *          handle_error();
 *
 * \param inserter The inserter.
 * \param select_list Comma separated list of expressions (encoded as UTF-8 string).
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_insert_computed_expressions(hyper_inserter_t* inserter, const char* select_list) HYPER_API_NOEXCEPT;

/**
 * Insert a Hyper binary chunk into a table.
 *
 * \param inserter  The inserter.
 * \param data_chunk  The data chunk to insert (encoded as Hyper binary).
 * \param bytes  The size of the data chunk in bytes.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_insert_chunk(hyper_inserter_t* inserter, const uint8_t* data_chunk, size_t bytes)
   HYPER_API_NOEXCEPT;

/**
 * Close the inserter. The associated connection will be ready afterwards.
 *
 * Sends all buffered rows to Hyper (if not canceled) and frees memory used by the `hyper_inserter_t` object.
 *
 * \param inserter  The inserter.
 * \param insert_data  Whether the data should be inserted. If false, the data is discarded and no changes are made to the table on Hyper side.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_close_inserter(hyper_inserter_t* inserter, bool insert_data) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Converting Data From/To Binary Format
 *
 * Functions to format data into the Hyper binary format.
 * @{
 */

/**
 * 128 bit integer type.
 */
typedef struct
{
   uint64_t data[2];
} hyper_data128_t;

/**
 * Write a null value for a nullable column of any type.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_null(uint8_t* target, size_t space) HYPER_API_NOEXCEPT;

/**
 * Write the hyper binary header. Every data chunk that is sent to hyper has to have the header as its first bytes.
 * See `hyper_inserter_insert_chunk`
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_header(uint8_t* target, size_t space) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 8 bits for a nullable column.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int8(uint8_t* target, size_t space, int8_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 8 bits for a column that cannot contain nulls.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int8_not_null(uint8_t* target, size_t space, int8_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 16 bits for a nullable column.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int16(uint8_t* target, size_t space, int16_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 16 bits for a column that cannot contain nulls.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int16_not_null(uint8_t* target, size_t space, int16_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 32 bits for a nullable column.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int32(uint8_t* target, size_t space, int32_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 32 bits for a column that cannot contain nulls.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int32_not_null(uint8_t* target, size_t space, int32_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 64 bits for a nullable column.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int64(uint8_t* target, size_t space, int64_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 64 bits for a column that cannot contain nulls.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_int64_not_null(uint8_t* target, size_t space, int64_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 128 bits for a nullable column.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_data128(uint8_t* target, size_t space, hyper_data128_t value) HYPER_API_NOEXCEPT;

/**
 * Write an integer with 128 bits for a column that cannot contain nulls.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_data128_not_null(uint8_t* target, size_t space, hyper_data128_t value) HYPER_API_NOEXCEPT;

/**
 * Write a variable-length binary string for a nullable column.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \param length The length of the value.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_varbinary(uint8_t* target, size_t space, const uint8_t* value, size_t length) HYPER_API_NOEXCEPT;

/**
 * Write a variable-length binary string for a column that cannot contain nulls.
 *
 * \param[out] target The target location to which the hyper binary should be written.
 * \param space The guaranteed number of bytes available at the target location.
 * \param value The value to write.
 * \param length The length of the value.
 * \return The number of bytes that have been written or would have been written if there was enough space.
 */
HYPER_API size_t hyper_write_varbinary_not_null(uint8_t* target, size_t space, const uint8_t* value, size_t length) HYPER_API_NOEXCEPT;

/**
 * Read an 8-bit integer.
 *
 * \param source The source data encoded as Hyper binary with length `sizeof(int8_t)`.
 * \return The read integer.
 */
HYPER_API int8_t hyper_read_int8(const uint8_t* source) HYPER_API_NOEXCEPT;

/**
 * Read a 16-bit integer.
 *
 * \param source The source data encoded as Hyper binary with length `sizeof(int16_t)`.
 * \return The read integer.
 */
HYPER_API int16_t hyper_read_int16(const uint8_t* source) HYPER_API_NOEXCEPT;

/**
 * Read a 32-bit integer.
 *
 * \param source The source data encoded as Hyper binary with length `sizeof(int32_t)`.
 * \return The read integer.
 */
HYPER_API int32_t hyper_read_int32(const uint8_t* source) HYPER_API_NOEXCEPT;

/**
 * Read a 64-bit integer.
 *
 * \param source The source data encoded as Hyper binary with length `sizeof(int64_t)`.
 * \return The read integer.
 */
HYPER_API int64_t hyper_read_int64(const uint8_t* source) HYPER_API_NOEXCEPT;

/**
 * Read a 128-bit integer.
 *
 * \param source The source data encoded as Hyper binary with length `sizeof(hyper_data128_t)`.
 * \return The read integer.
 */
HYPER_API hyper_data128_t hyper_read_data128(const uint8_t* source) HYPER_API_NOEXCEPT;

/**
 * Read a variable-length binary string.
 *
 * \param source The source data encoded as Hyper binary.
 * \return The read variable-length binary string.
 */
HYPER_API const uint8_t* hyper_read_varbinary(const uint8_t* source) HYPER_API_NOEXCEPT;

/** @} */

/**
 * \name Inserter Buffer
 * @{
 */

/**
 * A buffer object which manages writing individual values.
 */
typedef struct hyper_inserter_buffer_t hyper_inserter_buffer_t;

/**
 * Creates a buffer.
 *
 * \param inserter  The inserter.
 * \param table_definition  Table definition.
 * \param select_list Comma separated list of expressions for inserter(encoded as UTF-8 string).
 * \param [out] buffer  A pointer to the buffer object, or NULL if an error occurred.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_create_inserter_buffer(
   hyper_inserter_t* inserter, const hyper_table_definition_t* table_definition, const char* select_list, hyper_inserter_buffer_t** buffer) HYPER_API_NOEXCEPT;

/**
 * Sends buffered data to the inserter.
 *
 * \param buffer  The buffer.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_flush(hyper_inserter_buffer_t* buffer) HYPER_API_NOEXCEPT;

/**
 * Destroys the buffer object.
 *
 * \param buffer  The buffer.
 */
HYPER_API void hyper_inserter_buffer_destroy(hyper_inserter_buffer_t* buffer) HYPER_API_NOEXCEPT;

/**
 * Adds a NULL value.
 *
 * \param buffer  The buffer.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_null(hyper_inserter_buffer_t* buffer) HYPER_API_NOEXCEPT;

/**
 * Adds a boolean value.
 *
 * \param buffer  The buffer.
 * \param value   The value to add.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_bool(hyper_inserter_buffer_t* buffer, bool value) HYPER_API_NOEXCEPT;

/**
 * Adds an 16-bit integer value.
 *
 * \param buffer  The buffer.
 * \param value   The value to add.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_int16(hyper_inserter_buffer_t* buffer, int16_t value) HYPER_API_NOEXCEPT;

/**
 * Adds a 32-bit integer value.
 *
 * \param buffer  The buffer.
 * \param value   The value to add.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_int32(hyper_inserter_buffer_t* buffer, int32_t value) HYPER_API_NOEXCEPT;

/**
 * Adds a 64-bit integer value.
 *
 * \param buffer  The buffer.
 * \param value   The value to add.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_int64(hyper_inserter_buffer_t* buffer, int64_t value) HYPER_API_NOEXCEPT;

/**
 * Adds a double value.
 *
 * \param buffer  The buffer.
 * \param value   The value to add.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_double(hyper_inserter_buffer_t* buffer, double value) HYPER_API_NOEXCEPT;

/**
 * Adds a byte array.
 *
 * \param buffer  The buffer.
 * \param value   The array of bytes to add.
 * \param size    The size of the added array.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_binary(hyper_inserter_buffer_t* buffer, const uint8_t* value, size_t size)
   HYPER_API_NOEXCEPT;

/**
 * Adds a date array.
 *
 * \param buffer  The buffer.
 * \param year    The year.
 * \param month   The month, from 1 to 12.
 * \param day     The day of the month, from 1 to the number of days in the month.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_date(hyper_inserter_buffer_t* buffer, int32_t year, int16_t month, int16_t day)
   HYPER_API_NOEXCEPT;

/**
 * Adds a raw value in hyper binary format.
 *
 * \param buffer  The buffer.
 * \param value   The array of bytes to add.
 * \param size    The size of the added array.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_inserter_buffer_add_raw(hyper_inserter_buffer_t* buffer, const uint8_t* value, size_t size)
   HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Date, Time, Interval Data Types
 * @{
 */

/**
 * A type that represents dates.
 *
 * Example: 2019-11-02
 */
typedef uint32_t hyper_date_t;

/**
 * The components of a date.
 */
typedef struct
{
   int32_t year;
   int16_t month;
   int16_t day;
} hyper_date_components_t;

/**
 * A type that represents the time of day with microsecond precision.
 *
 * Example: 12:34:56.789
 */
typedef uint64_t hyper_time_t;

/**
 * The components of a point in time.
 */
typedef struct
{
   int8_t hour;
   int8_t minute;
   int8_t second;
   int32_t microsecond;
} hyper_time_components_t;

/**
 * A type that represents a point in time.
 *
 * Example: 2019-11-02 12:34:56.789
 */
typedef uint64_t hyper_timestamp_t;

/**
 * A type that represents a time interval with microsecond precision.
 *
 * Example: 1 years 2 months 3 days 4 hours 5 minutes 6 seconds 789 microseconds
 */
typedef hyper_data128_t hyper_interval_t;

/**
 * The components of an interval.
 */
typedef struct
{
   int32_t years;
   int32_t months;
   int32_t days;
   int32_t hours;
   int32_t minutes;
   int32_t seconds;
   int32_t microseconds;
} hyper_interval_components_t;

/**
 * Decode a date into its components.
 *
 * \param date The date to decode.
 * \return The date components.
 */
HYPER_API hyper_date_components_t hyper_decode_date(hyper_date_t date) HYPER_API_NOEXCEPT;

/**
 * Encode a date from year, month, and day components.
 *
 * \param components The date components.
 * \return The resulting date.
 */
HYPER_API hyper_date_t hyper_encode_date(hyper_date_components_t components) HYPER_API_NOEXCEPT;

/**
 * Decode a point in time into its components.
 *
 * \param time The point in time to decode.
 * \return The time components.
 */
HYPER_API hyper_time_components_t hyper_decode_time(hyper_time_t time) HYPER_API_NOEXCEPT;

/**
 * Encode a point in time from hour, minute, second, and microsecond components.
 *
 * \param components The time components.
 * \return The resulting point in time.
 */
HYPER_API hyper_time_t hyper_encode_time(hyper_time_components_t components) HYPER_API_NOEXCEPT;

/**
 * Decode an interval into its components.
 *
 * \param interval The interval to decode.
 * \return The interval components.
 */
HYPER_API hyper_interval_components_t hyper_decode_interval(hyper_interval_t interval) HYPER_API_NOEXCEPT;

/**
 * Encode an interval from the given number of years, months, days, hours, minutes, seconds, and microseconds.
 *
 * \param components The interval components.
 * \return The resulting interval.
 */
HYPER_API hyper_interval_t hyper_encode_interval(hyper_interval_components_t components) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Numeric Data Type
 * @{
 */

/**
 * Parse a numeric value from a string representation.
 *
 * \param[in,out] iter  A pointer to a pointer to the beginning of the string.
 * \param limit  Pointer to past the end of the string.
 * \param precision  Resulting precision of the numeric.
 * \param scale  Resulting scale of the numeric.
 * \param[out] result  Pointer to a location where the result will be stored, if the call is successful.
 * \return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_parse_numeric(const char** iter, const char* limit, uint32_t precision, uint32_t scale, int64_t* result)
   HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Data copy
 * @{
 */

/**
 * Copy the given buffer.
 *
 * Returns an error if the connection is not ready.
 *
 * @param connection The connection.
 * @param buffer A buffer containing the data that will be sent.
 * @param size The size in bytes of the buffer that will be sent.
 * @return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_copy_data(hyper_connection_t* connection, const uint8_t* buffer, int size) HYPER_API_NOEXCEPT;

/**
 * End the copy statement.
 *
 * Returns an error if the connection is not ready.
 *
 * @param connection The connection.
 * @return
 *    Returns NULL on success; otherwise a `hyper_error_t*` object indicating the error.
 *    The caller _must_ handle this error and eventually call `hyper_error_destroy()` to free any associated memory.
 */
HYPER_API HYPER_API_NODISCARD hyper_error_t* hyper_copy_end(hyper_connection_t* connection) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Logging
 * @{
 */

// Log level.
/**
 * An enumeration of logging levels of errors.
 */
typedef enum {
   /** Logging level for fine-grained events that are useful for debugging the application. */
   HYPER_LOG_LEVEL_TRACE = 0,
   /** Logging level for informational events that indicate progess of the application. */
   HYPER_LOG_LEVEL_INFO,
   /** Logging level for potential harmful events that indicate problems affecting progress of the application. */
   HYPER_LOG_LEVEL_WARNING,
   /** Logging level for error events that prevents normal application progress. */
   HYPER_LOG_LEVEL_ERROR,
   /** Logging level for very severe error events that might cause the application to terminate. */
   HYPER_LOG_LEVEL_FATAL
} hyper_log_level_t;

/**
 * Pointer to a log function.
 * The function is supposed to log an event with the given log_level, topic, and json value.
 * It must be thread-safe and noexcept.
 */
typedef void (*hyper_log_function_t)(hyper_log_level_t log_level, const char* topic, const char* json_value, void* context);

/**
 * Sets the log function to be used by the application.
 * Default log function is set to NULL
 *
 * \param log_function The new log function to be used.
 * \param context Custom context to be used by the log function.
 * \return The previous log function.
 */
HYPER_API hyper_log_function_t hyper_log_set_log_function(hyper_log_function_t log_function, void* context) HYPER_API_NOEXCEPT;

/**
 * Sets the log level for the application.
 * Default log level is HYPER_LOG_LEVEL_INFO
 *
 * \param log_level The log level.
 * \return The previous log level.
 */
HYPER_API hyper_log_level_t hyper_log_set_log_level(hyper_log_level_t log_level) HYPER_API_NOEXCEPT;

/**
 * Default log function to log an event to `stdcerr`.
 * Use `hyper_log_set_log_function(hyper_default_log_function)` to enable it.
 *
 * \param log_level Log level for the logging event
 * \param topic Topic under which the event is logged
 * \param json_value Log contents
 * \param context Custom context provided when setting the log function
 */
HYPER_API void hyper_default_log_function(hyper_log_level_t log_level, const char* topic, const char* json_value, void* context);

/**
 * Logs an event using the current log function.
 *
 * \param log_level Log level for the logging event
 * \param topic Topic under which the event is logged
 * \param json_value Log contents
 */
HYPER_API void hyper_log_event(hyper_log_level_t log_level, const char* topic, const char* json_value) HYPER_API_NOEXCEPT;

/** @} */

//---------------------------------------------------------------------------

/**
 * \name Type OID constants.
 * @{
 */

/**
 * OID constants.
 */
enum {
   /** OID of the type HYPER_BOOL */
   HYPER_OID_BOOL = 16,
   /** OID of the type HYPER_BIG_INT */
   HYPER_OID_BIG_INT = 20,
   /** OID of the type HYPER_SMALL_INT */
   HYPER_OID_SMALL_INT = 21,
   /** OID of the type HYPER_INT */
   HYPER_OID_INT = 23,
   /** OID of the type HYPER_NUMERIC */
   HYPER_OID_NUMERIC = 1700,
   /** OID of the type HYPER_DOUBLE */
   HYPER_OID_DOUBLE = 701,
   /** OID of the type HYPER_OID */
   HYPER_OID_OID = 26,
   /** OID of the type HYPER_BYTE_A */
   HYPER_OID_BYTE_A = 17,
   /** OID of the type HYPER_TEXT */
   HYPER_OID_TEXT = 25,
   /** OID of the type HYPER_VARCHAR */
   HYPER_OID_VARCHAR = 1043,
   /** OID of the type HYPER_CHAR with length modifier other than 1 */
   HYPER_OID_CHAR = 1042,
   /** OID of the type HYPER_CHAR with length modifier of 1 */
   HYPER_OID_CHAR1 = 18,
   /** OID of the type HYPER_JSON */
   HYPER_OID_JSON = 114,
   /** OID of the type HYPER_DATE */
   HYPER_OID_DATE = 1082,
   /** OID of the type HYPER_INTERVAL */
   HYPER_OID_INTERVAL = 1186,
   /** OID of the type HYPER_TIME */
   HYPER_OID_TIME = 1083,
   /** OID of the type HYPER_TIMESTAMP */
   HYPER_OID_TIMESTAMP = 1114,
   /** OID of the type HYPER_TIMESTAMP_TZ */
   HYPER_OID_TIMESTAMP_TZ = 1184,
   /** OID of the type HYPER_GEOGRAPHY */
   HYPER_OID_GEOGRAPHY = 5003,
};

/** @} */

//---------------------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif
