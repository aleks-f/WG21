// This is a mock-up, stripped-down implementation of the two-class 
// design as described in N3565 ("IP ADDRESS DESIGN CONSTRAINTS",
// http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2013/n3565.pdf),
// paragraph 3.2.2.
// 
// Classes are tentatively named ip::address_v4 and ip::address_v6, with
// addition of typedef ip::address.
// 
// The main purpose is to show what would a two-class approach look like,
// especially in the concepts context (eg. "requires IPAddress(AddressType)")

#include <iostream>
#include <stdexcept>
#include <string>

namespace std {
namespace net {
namespace ip {


class address_v6;


class address_v4
{
public:

	address_v4(const std::string& address = "0.0.0.0") : _address(address)
	{
		if ("0.0.0.0" == address) std::cout << "default ";
		std::cout << "construct IPv4" << std::endl;
	}

	address_v4(const address_v4& addrv4)
	{
		_address = addrv4.to_string();
		std::cout << "copy construct IPv4" << std::endl;
	}

	address_v4(const address_v6& addrv6);
		/// See below for implementation.

	const address_v4& operator = (const address_v4& addrv4)
	{
		_address = addrv4.to_string();
		std::cout << "assign IPv4 to IPv4" << std::endl;
	}

	const address_v4& operator = (const address_v6& addrv6);
		/// Throws if addrv6 is not convertible to IPv4, see below
		/// for implementation.

	std::string to_string() const
	{
		return _address;
	}

	// other members ...

private:
	std::string _address;
	// unsigned char _address[4];
};


class address_v6
{
public:

	address_v6(const std::string& address = "::") : _address(address)
	{
		if (is_v4_mapped()) std::cout << "default ";
		std::cout << "construct IPv6";
		if (is_v4_mapped()) std::cout << " from IPv4";
		std::cout << std::endl;
	}

	address_v6(const address_v6& addrv6)
	{
		_address = addrv6.to_string();
		std::cout << "copy construct IPv6" << std::endl;
	}

	address_v6(const address_v4& addrv4) : _address(addrv4.to_string())
	{
		std::cout << "construct IPv6 from IPv4" << std::endl;
	}

	const address_v6& operator = (const address_v6& addrv6)
	{
		_address = addrv6.to_string();
		std::cout << "assign IPv6 to IPv6" << std::endl;
		return *this;
	}

	const address_v6& operator = (const address_v4& addrv4)
	{
		_address = addrv4.to_string();
		std::cout << "assign IPv4 to IPv6" << std::endl;
		return *this;
	}

	std::string to_string() const
	{
		std::string ret;
		if (is_v4_mapped()) ret.append("::");
		ret += _address;
		return ret;
	}

	bool is_v4_mapped() const
	{
		return _address.find_first_of('.') != std::string::npos;
	}

	// other members ...

private:
	std::string _address;
	// unsigned char _address[16];
};


const address_v4& address_v4::operator = (const address_v6& addrv6)
{
	if (!addrv6.is_v4_mapped()) throw std::range_error("cannot convert IPv6 address to IPv4");
	_address = addrv6.to_string();
	std::string::size_type pos = _address.find_last_of(':') + 1;
	if (pos != std::string::npos) _address.assign(_address.begin() + pos, _address.end());
	std::cout << "assign IPv6 to IPv4" << std::endl;
	return *this;
}


address_v4::address_v4(const address_v6& addrv6)
{
	*this = addrv6;
	std::cout << "construct IPv4 from IPv6" << std::endl;
}


typedef address_v6 address;


// Following function demonstrates transparent use of
// ip::address_v4, ip::address_v6 and ip::address,
// with example of what a hypothetical IPAddress concept
// would look like.
//
// Potential concept (with 'IPAddress' concept) versions:
// 
// template <IPAddress AddressType>
// 
// or:
// 
// template <typename AddressType>
//   requires IPAddress(AddressType)
//
// Based on N3580 ("Concepts Lite: Constraining Templates with Predicates").
// http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2013/n3580.pdf
//
template <typename AddressType>
void print_address(const AddressType& addr)
{
	std::cout << "print_address: " << addr.to_string() << std::endl;
}


} } } // namespace std::net::ip


int main()
{
	using namespace std::net::ip;

	address_v4 ipv4; // address_v4::address_v4("0.0.0.0")
	address_v6 ipv6; // address_v6::address_v6("::")
	address    ip;   // address_v6::address_v6("::")

	print_address (ipv4);
	print_address (ipv6);
	print_address (ip);

	ipv6 = ipv4; // ok, IPv4 => IPv6 always succeeds for valid IPv4 address
	print_address (ipv6);
	ip = ipv6;   // ok, address is address_v6
	print_address (ip);
	ip = ipv4;   // ok, address is address_v6 and can receive IPv4
	print_address (ip);

	// make explicitly an IPv6
	ipv6 = address_v6("2001:db8:85a3::8a2e:370:7334");
	print_address (ipv6);
	// then try to assign to ipv4 ...
	try { ipv4 = ipv6; } // throws, ipv6 is NEITHER ipv4-mapped NOR (obsoleted) ipv4-compatible
	catch (std::range_error& er) { std::cout << er.what() << std::endl; }

	// make explicitly an IPv4-mapped or -compatible IPv6
	address ipv4v6("192.168.0.1");
	print_address (ipv4v6);
	ipv4 = address_v6(ipv4v6); // ok, now IPv6 address holds IPv4-mapped
	print_address (ipv4);

	return 0;
}