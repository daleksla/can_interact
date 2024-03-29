#ifndef CAN_INTERACT_HH
#define CAN_INTERACT_HH
#pragma once

#if !(__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))
#error The can_interact CXX API needs at least a C++11 compliant compiler
#endif // !(__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))

#include <vector>
#include <type_traits>
#include <array>
#include <string>
#include <stdexcept>
#include <cstddef>
#include <cstdint>

#include <unistd.h>
#include <errno.h>

#include "can_interact.h"

/**
 * @brief CXX API (C++11) of can_interact C library code used to usefully read and write to CAN bus
 * Functionality has been written to provide both functions working with relevant native constructs (i.e. C-style pointers) and suitable STL containers
 * However templating has been used to get rid of unnecessary usage of generic pointers and manual specifications of value types & lengths, as-well as SFINAE
 *
 * Please refer to docstring of native library to understand erroneous possibilities of wrapper functions - decided not to detail too much here as this is a mere wrapper and its behaviour is reliant on native library calls
 * For declarations for the native C library, see can_interact.h
 */

extern int errno ;

namespace can_interact {

	class CAN {
		/**
		  * @brief CAN (class) - class to manage Control Area Networks (CAN)
		  * This includes establishing & maintaining a connection and sending & receiving messages
		  */
		private:
			int _socket ;

		public:
			/**
			  * @brief CAN (constructor) (overload) - initialises CAN connection
			  * @param const std::string& - name of device
			  * @throws std::runtime_exception - in case can_interact_* functionality returns non-zero error
			  */
			CAN(const std::string&) noexcept(false) ;

			/**
			  * @brief CAN (constructor) (overload) - adopts and works with existing socket
			  * @param const int - existing, initialised socket
			  */
			CAN(const int) noexcept ;

			/**
			  * @brief CAN (copy constructor) - copies properties of existing socket ID
			  * @param const CAN& - lvalue reference to existing CAN class object
			  * @throws std::invalid_argument - if properties of CAN object to be copied / duplicated are not initialised
			  * @throws std::runtime_error - if error is encountered duplicating seemingly valid internal socket
			  */
			CAN(const CAN&) noexcept(false) ;

			/**
			  * @brief operator= (copy assignment) -  copies properties of existing socket ID
			  * @param const CAN& - lvalue reference to existing CAN class object
			  * @throws std::invalid_argument - if properties of CAN object to be copied / duplicated are not initialised
			  * @throws std::runtime_error - if error is encountered duplicating seemingly valid internal socket
			  * @return CAN& - reference to created CAN object
			  */
			CAN& operator=(const CAN&) noexcept(false) ;

			/**
			  * @brief CAN (move constructor) - copies over internal socket ID and nullifies previous
			  * @param CAN&& - rvalue reference to CAN class object
			  */
			CAN(CAN&&) noexcept ;

			/**
			  * @brief operator= (move assignment) - copies over internal socket ID and nullifies previous
			  * @param CAN&& - rvalue reference to CAN class object
			  * @return CAN& - reference to created CAN object
			  */
			CAN& operator=(CAN&&) noexcept ;

			/**
			  * @brief socket (overload) - getter to return socket being internally maintained
			  * @return int - socket
			  */
			int socket() const noexcept ;

			/**
			  * @brief socket (overload) - setter to set socket being internally maintained
			  * @param const int - socket for current object to work with
			  */
			void socket(const int) noexcept ;

			/**
			  * @brief filter (overload) - sets up kernel level filtering to internal CAN sockets
			  * @param const std::uint32_t* - const array of (hex) ids to request from kernel filter
			  * @param const size_t - length of array of hex ids to filter for
			  * @throws std::runtime_exception - in case can_interact_* functionality returns non-zero error (and errors reported by errno)
			  */
			void filter(const std::uint32_t*, const std::size_t) noexcept(false) ;

			/**
			  * @brief filter (overload) - sets up kernel level filtering to internal CAN sockets
			  * @param const std::vector<std::uint32_t>& - name of device
			  * @throws std::runtime_exception - in case can_interact_* functionality returns non-zero error (and errors reported by errno)
			  */
			void filter(const std::vector<std::uint32_t>&) noexcept(false) ;

			/**
			  * @brief filter (overload) - sets up kernel level filtering to internal CAN sockets
			  * @tparam std::size_t SIZE - length of array
			  * @param const std::array<std::uint32_t, SIZE>& - name of device
			  * @throws std::runtime_exception - in case can_interact_* functionality returns non-zero error (and errors reported by errno)
			  */
			template<std::size_t SIZE>
			void filter(const std::array<std::uint32_t, SIZE>&) noexcept(false) ;

			/**
			  * @brief frame (overload) - returns frame from CAN
			  * @return can_frame - LINUX CAN frame struct
			  * @throws std::runtime_exception - in case can_interact_* functionality returns non-zero error (and errors reported by errno)
			  */
			can_frame frame() const noexcept(false) ;

			/**
			  * @brief frame (overload) - sends frame from CAN
			  * This method expects a LINUX can_frame struct
			  * You may use can_interact::encode to assemble the struct for you
			  *
			  * @param const can_frame& - LINUX CAN frame struct
			  *
			  * @throws std::runtime_exception - in case can_interact_* functionality returns non-zero error (and errors reported by errno)
			  */
			void frame(const can_frame&) const noexcept(false) ;

			/**
			  * @brief ~CAN (destructor) - frees CAN socket connections
			  * TODO closing COULD fail but this method has no way of throwing exception
			  * For now, I reset errno to 0 and then, if its 0 (success) or not (else) and consult errno
			  */
			~CAN() 	noexcept ;

			/* Below are defaulted and deleted methods */
			CAN() noexcept = delete ;
	} ;

	// delete general cases, we only want 3 cases: long unsigned, long int, double, implemented below
	template<typename T>
	T decode(const can_frame&, const can_interact_endianness) noexcept(false) = delete ;

	/**
	  * @brief decode - decodes bytes
	  *
	  * @tparam std::uint64_t - what data-type the byte payload will be encoded into and returned as a result. Will internally trigger DATA_TYPE_UNSIGNED for decoding
	  *
	  * @param const cam_frame& - reference to LINUX can_frame
	  *
	  * @param const can_interact_endianness - enum indicating the output byte order
	  * can_interact_endianness::ENDIAN_LITTLE is little, can_interact_endianness::ENDIAN_BIG is big
	  *
	  * @throws std::invalid_argument - in case can_interact_* functionality returns non-zero error (due to badly sized input)
	  *
	  * @return std::uint64_t - compiled payload
	  */
	template<>
	std::uint64_t decode<std::uint64_t>(const can_frame&, const can_interact_endianness) noexcept(false) ;

	/**
	  * @brief decode - decodes bytes
	  *
	  * @tparam std::int64_t - what data-type the byte payload will be encoded into and returned as a result. Will internally trigger DATA_TYPE_SIGNED for decoding
	  *
	  * @param const cam_frame& - reference to LINUX can_frame
	  *
	  * @param const can_interact_endianness - enum indicating the output byte order
	  * can_interact_endianness::ENDIAN_LITTLE is little, can_interact_endianness::ENDIAN_BIG is big
	  *
	  * @throws std::invalid_argument - in case can_interact_* functionality returns non-zero error (due to badly sized input)
	  *
	  * @return std::int64_t - compiled payload
	  */
	template<>
	std::int64_t decode<std::int64_t>(const can_frame&, const can_interact_endianness) noexcept(false) ;

	/**
	  * @brief decode - decodes bytes
	  *
	  * @tparam double - what data-type the byte payload will be encoded into and returned as a result. Will internally trigger DATA_TYPE_FLOAT for decoding
	  *
	  * @param const cam_frame& - reference to LINUX can_frame
	  *
	  * @param const can_interact_endianness - enum indicating the output byte order
	  * can_interact_endianness::ENDIAN_LITTLE is little, can_interact_endianness::ENDIAN_BIG is big
	  *
	  * @throws std::invalid_argument - in case can_interact_* functionality returns non-zero error (due to badly sized input)
	  *
	  * @return double - compiled payload
	  */
	template<>
	double decode<double>(const can_frame&, const can_interact_endianness) noexcept(false) ;

	/**
	  * @brief encode - encodes values as bytes
	  *
	  * @tparam typename F - generic type providing it is a float
	  *
	  * @param const canid_t - ID of to-be-sent CAN frame
	  *
	  * @param const F - value to encode
	  *
	  * @param const can_interact_endianness - enum indicating the output byte order
	  * can_interact_endianness::ENDIAN_LITTLE is little, can_interact_endianness::ENDIAN_BIG is big
	  *
	  * @throws std::invalid_argument - in case can_interact_* functionality returns non-zero error (due to badly sized input)
	  *
	  * @return can_frame - LINUX can frame struct ready to be sent
	  */
	template<typename F, typename std::enable_if<std::is_floating_point<F>::value, bool>::type = true>
	can_frame encode(const canid_t, const F, const can_interact_endianness) noexcept(false) ;

	/**
	  * @brief encode - encodes values as bytes
	  * @tparam typename S - generic type providing it is a signed integer
	  * @param const S - value to encode
	  *
	  * @param const can_interact_endianness - enum indicating the output byte order
	  * can_interact_endianness::ENDIAN_LITTLE is little, can_interact_endianness::ENDIAN_BIG is big
	  *
	  * @throws std::invalid_argument - in case can_interact_* functionality returns non-zero error (due to badly sized input)
	  *
	  * @return can_frame - LINUX can frame struct ready to be sent
	  */
	template<typename S, typename std::enable_if<std::is_integral<S>::value && std::is_signed<S>::value, bool>::type = true>
	can_frame encode(const canid_t, const S, const can_interact_endianness) noexcept(false) ;

	/**
	  * @brief encode - encodes values as bytes
	  *
	  * @tparam typename U - generic type providing it is a unsigned integer
	  *
	  * @param const U - value to encode
	  *
	  * @param const can_interact_endianness - enum indicating the output byte order
	  * can_interact_endianness::ENDIAN_LITTLE is little, can_interact_endianness::ENDIAN_BIG is big
	  *
	  * @throws std::invalid_argument - in case can_interact_* functionality returns non-zero error (due to badly sized input)
	  *
	  * @return can_frame - LINUX can frame struct ready to be sent
	  */
	template<typename U, typename std::enable_if<std::is_integral<U>::value && !std::is_signed<U>::value,bool>::type = true>
	can_frame encode(const canid_t, const U, const can_interact_endianness) noexcept(false) ;

}

can_interact::CAN::CAN(const std::string& device_name) noexcept(false)
{
	const int res = can_interact_init(&this->_socket, device_name.c_str()) ;
	if(res != 0)
	{
		throw std::runtime_error(std::string{"Errno "} + std::to_string(res)) ;
	}
}

can_interact::CAN::CAN(const int socket) noexcept : _socket(socket) {}

can_interact::CAN::CAN(const can_interact::CAN& can) noexcept(false)
{
	if(can._socket == -1)
	{
		const std::string msg = "Error duplicating CAN socket as origin is not initialised" ;
		throw std::invalid_argument(msg) ;
	}
	this->_socket = dup(can._socket) ;
	if(this->_socket == -1)
	{
		const std::string msg = std::string{"Errno "} + std::to_string(errno) ;
		throw std::runtime_error(msg) ;
	}
}

can_interact::CAN& can_interact::CAN::operator=(const CAN& can) noexcept(false)
{
	if(can._socket == -1)
	{
		const std::string msg = "Error duplicating CAN socket as origin is not initialised" ;
		throw std::invalid_argument(msg) ;
	}
	this->_socket = dup(can._socket) ;
	if(this->_socket == -1)
	{
		const std::string msg = std::string{"Errno "} + std::to_string(errno) ;
		throw std::runtime_error(msg) ;
	}
	return *this ;
}

can_interact::CAN::CAN(can_interact::CAN&& can) noexcept
{
	this->_socket = can._socket ;
	can._socket = -1 ;
}

can_interact::CAN& can_interact::CAN::operator=(CAN&& can) noexcept
{
	this->_socket = can._socket ;
	can._socket = -1 ;
	return *this ;
}

int can_interact::CAN::socket() const noexcept
{
	return this->_socket ;
}

void can_interact::CAN::socket(const int socket) noexcept
{
	this->_socket = socket ;
}

void can_interact::CAN::filter(const std::uint32_t* filter_ids, const size_t len) noexcept(false)
{
	int res = can_interact_filter(filter_ids, len, &this->_socket) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Errno "} + std::to_string(res) ;
		throw std::runtime_error(msg) ;
	}
}

void can_interact::CAN::filter(const std::vector<std::uint32_t>& filter_ids) noexcept(false)
{
	this->filter(filter_ids.data(), filter_ids.size()) ;
}

template<std::size_t SIZE>
void can_interact::CAN::filter(const std::array<std::uint32_t, SIZE>& filter_ids) noexcept(false)
{
	this->filter(filter_ids.data(), filter_ids.size()) ;
}

can_frame can_interact::CAN::frame() const noexcept(false)
{
	can_frame frame ;
	const int res = can_interact_get_frame(&frame, &this->_socket) ;
	if(res != 0)
	{
		throw std::runtime_error(std::string{"Errno "} + std::to_string(res)) ;
	}
	return frame ;
}

void can_interact::CAN::frame(const can_frame& frame) const noexcept(false)
{
	const int res = can_interact_send_frame(&frame, &this->_socket) ;
	if(res != 0)
	{
		throw std::runtime_error(std::string{"Errno "} + std::to_string(res)) ;
	}
}

can_interact::CAN::~CAN() noexcept
{
	errno = 0 ; // TODO find better way to report (and not report) potential errors
	if(this->_socket != -1)
	{
		can_interact_fini(&this->_socket) ;
	}
}

template<>
std::uint64_t can_interact::decode<std::uint64_t>(const can_frame& frame, const can_interact_endianness byte_order) noexcept(false)
{
	std::uint64_t val ;
	const int res = can_interact_decode(&frame, can_interact_data_type::DATA_TYPE_UNSIGNED, byte_order, &val) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Error "} + std::to_string(res) + std::string{" indicates "} + std::string{"payload provided is too small or large (below 0 or above 8 bytes)"} ;
		throw std::invalid_argument(msg) ;
	}

	return val ;
}

template<>
std::int64_t can_interact::decode<std::int64_t>(const can_frame& frame, const can_interact_endianness byte_order) noexcept(false)
{
	std::int64_t val ;
	const int res = can_interact_decode(&frame, can_interact_data_type::DATA_TYPE_SIGNED, byte_order, &val) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Error "} + std::to_string(res) + std::string{" indicates "} + std::string{"payload provided is too small or large (below 0 or above 8 bytes)"} ;
		throw std::invalid_argument(msg) ;
	}
	
	return val ;
}

template<>
double can_interact::decode<double>(const can_frame& frame, const can_interact_endianness byte_order) noexcept(false)
{
	double val ;
	const int res = can_interact_decode(&frame, can_interact_data_type::DATA_TYPE_FLOAT, byte_order, &val) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Error "} + std::to_string(res) + std::string{" indicates "} + (res == 1 ? std::string{"payload provided is too small or large (below 0 or above 8 bytes)"} : std::string{"payload for intended floating point value is neither 4 or 8 bytes"}) ;
		throw std::invalid_argument(msg) ;
	}
	
	return val ;
}

template<typename F, typename std::enable_if<std::is_floating_point<F>::value, bool>::type = true>
can_frame can_interact::encode(const canid_t id, const F val, const can_interact_endianness endianness) noexcept(false)
{
	can_frame frame ;
	const int res = can_interact_encode(id, &val, sizeof(val), can_interact_data_type::DATA_TYPE_SIGNED, endianness, &frame) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Error "} + std::to_string(res) + std::string{" indicates value to encode is of a wrong-size (must be 4/8 bytes in length)"} ;
		throw std::invalid_argument(msg) ;
	}
	return frame ;
}

template<typename S, typename std::enable_if<std::is_integral<S>::value && std::is_signed<S>::value, bool>::type = true>
can_frame can_interact::encode(const canid_t id, const S val, const can_interact_endianness endianness) noexcept(false)
{
	can_frame frame ;
	const int res = can_interact_encode(id, &val, sizeof(val), can_interact_data_type::DATA_TYPE_SIGNED, endianness, &frame) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Error "} + std::to_string(res) + std::string{" indicates value to encode is of a wrong-size (must be 1-8 bytes in length)"} ;
		throw std::invalid_argument(msg) ;
	}
	return frame ;
}

template<typename U, typename std::enable_if<std::is_integral<U>::value && !std::is_signed<U>::value, bool>::type = true>
can_frame can_interact::encode(const canid_t id, const U val, const can_interact_endianness endianness) noexcept(false)
{
	can_frame frame ;
	const int res = can_interact_encode(id, &val, sizeof(val), can_interact_data_type::DATA_TYPE_UNSIGNED, endianness, &frame) ;
	if(res != 0)
	{
		const std::string msg = std::string{"Error "} + std::to_string(res) + std::string{" indicates value to encode is of a wrong-size (must be 1-8 bytes in length)"} ;
		throw std::invalid_argument(msg) ;
	}
	return frame ;
}

#endif // CAN_INTERACT_HH
