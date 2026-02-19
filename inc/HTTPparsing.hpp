/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   HTTPparsing.hpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/19 16:36:37 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/19 16:39:04 by mde-beer            ########   odam.nl   */
/*                                                                            */
/*   —————No norm compliance?——————                                           */
/*   ⠀⣞⢽⢪⢣⢣⢣⢫⡺⡵⣝⡮⣗⢷⢽⢽⢽⣮⡷⡽⣜⣜⢮⢺⣜⢷⢽⢝⡽⣝                                           */
/*   ⠸⡸⠜⠕⠕⠁⢁⢇⢏⢽⢺⣪⡳⡝⣎⣏⢯⢞⡿⣟⣷⣳⢯⡷⣽⢽⢯⣳⣫⠇                                           */
/*   ⠀⠀⢀⢀⢄⢬⢪⡪⡎⣆⡈⠚⠜⠕⠇⠗⠝⢕⢯⢫⣞⣯⣿⣻⡽⣏⢗⣗⠏⠀                                           */
/*   ⠀⠪⡪⡪⣪⢪⢺⢸⢢⢓⢆⢤⢀⠀⠀⠀⠀⠈⢊⢞⡾⣿⡯⣏⢮⠷⠁⠀⠀⠀                                           */
/*   ⠀⠀⠀⠈⠊⠆⡃⠕⢕⢇⢇⢇⢇⢇⢏⢎⢎⢆⢄⠀⢑⣽⣿⢝⠲⠉⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⠀⡿⠂⠠⠀⡇⢇⠕⢈⣀⠀⠁⠡⠣⡣⡫⣂⣿⠯⢪⠰⠂⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⡦⡙⡂⢀⢤⢣⠣⡈⣾⡃⠠⠄⠀⡄⢱⣌⣶⢏⢊⠂⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⢝⡲⣜⡮⡏⢎⢌⢂⠙⠢⠐⢀⢘⢵⣽⣿⡿⠁⠁⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⠨⣺⡺⡕⡕⡱⡑⡆⡕⡅⡕⡜⡼⢽⡻⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⣼⣳⣫⣾⣵⣗⡵⡱⡡⢣⢑⢕⢜⢕⡝⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⣴⣿⣾⣿⣿⣿⡿⡽⡑⢌⠪⡢⡣⣣⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⡟⡾⣿⢿⢿⢵⣽⣾⣼⣘⢸⢸⣞⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ⠀⠀⠀⠀⠁⠇⠡⠩⡫⢿⣝⡻⡮⣒⢽⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀                                           */
/*   ——————————————————————————————                                           */
/* ************************************************************************** */

#include <Parser.hpp>
// plumbing
namespace HTTPparsing {
	template <typename T>
	using Func = std::function<T>;
	using String = std::string;

	extern const Parser<char>	tchar;
	extern const Parser<char>	vchar;
	extern const Parser<char>	obsText;
	extern const Parser<char>	fieldVchar;
	extern const Parser<String>	ows;
	extern const Parser<String>	rws;
	extern const Parser<String>	crlf;
	extern const Parser<String>	token;
	extern const Parser<String>	pctEncoded;
	extern const Parser<char>	subDelims;
	extern const Parser<String>	method;
	extern const Parser<String>	fieldContent;
	extern const Parser<String>	fieldValue;
	extern const Parser<String>	fieldName;
	extern const Parser<String>	fieldLine;
	extern const Parser<std::vector<String>>	fieldLines;
	extern const Parser<String>	httpVersion;
	extern const Parser<char>	unreserved;
	extern const Parser<String>	pchar;
	extern const Parser<String>	query;
	extern const Parser<String>	segmentNz;
	extern const Parser<String>	segment;
	extern const Parser<String>	pathEmpty;
	extern const Parser<String>	pathRootless;
	extern const Parser<String>	pathAbsolute;
	extern const Parser<String>	pathAbempty;
	extern const Parser<String>	decOctet;
	extern const Parser<String>	h16;
	extern const Parser<String>	ipv4address;
	extern const Parser<String>	ls32;
	extern const Parser<String>	port;
	extern const Parser<String>	regName;
	extern const Parser<String>	ipvfuture;
	extern const Parser<String>	ipv6address;
	extern const Parser<String>	ipLiteral;
	extern const Parser<String>	host;
	extern const Parser<String>	uriHost;
	extern const Parser<String>	userinfo;
	extern const Parser<String>	authority;
	extern const Parser<String>	hierPart;
	extern const Parser<String>	scheme;
	extern const Parser<String>	absoluteURI;
	extern const Parser<String>	absolutePath;
	extern const Parser<String>	reasonPhrase;
	extern const Parser<String>	statusCode;
	extern const Parser<String>	statusLine;
	extern const Parser<String>	asteriskForm;
	extern const Parser<String>	authorityForm;
	extern const Parser<String>	absoluteForm;
	extern const Parser<String>	originForm;
	extern const Parser<String>	requestTarget;
	extern const Parser<String>	requestLine;
	extern const Parser<String>	startLine;
	extern const Parser<String>	messageBody;
}
// [NOTE] taken from RFC-editor.org
// the variables defined under HTTPparsing are based entirely on the following
// Augmented Backus-naur form of an http message:
// HTTP-message		=	start-line '\r''\n'
// 						*( field-line '\r''\n' )
// 						'\r''\n'
// 						[ message-body ]
//
// start-line		=	request-line
// 					|	status-line
//
// request-line		=	method ' ' request-target ' ' HTTP-version
//
// method			=	token
// token			=	1*tchar
// tchar			=	'!' | '#' | '$' | '%' | '&' | '\'' | '*' | '+' | '-'
// 					|	'.' | '^' | '_' | '`' | '|' | '~' | DIGIT | ALPHA
//
// request-target	=	origin-form
// 					|	absolute-form
// 					|	authority-form
// 					|	asterisk-form
//
// origin-form		=	absolute-path [ '?' query ]
// absolute-path	=	1*( '/' segment )
// segment			=	*pchar
// pchar			=	unreserved | pct-encoded | sub-delims | ':' | '@'
// unreserved		=	ALPHA | DIGIT | '-' | '.' | '_' | '~'
// pct-encoded		=	'%' HEXDIG HEXDIG
// sub-delims		=	'!' | '$' | '&' | '\'' | '(' | ')' | '*' | '+' | ','
// 					|	';' | '='
// query			=	*( pchar | '/' | '?' )
//
// absolute-form	=	absolute-URI
// absolute-URI		=	scheme ':' hier-part [ '?' query ]
// scheme			=	ALPHA *( ALPHA | DIGIT | '+' | '-' | '.' )
// hier-part		=	'/''/' authority path-abempty
// 					|	path-absolute
// 					|	path-rootless
// 					|	path-empty
// authority		=	[ userinfo '@' ] host [ ':' port ]
// userinfo			=	*( unreserved | pct-encoded | sub-delims | ':' )
// host				=	IP-literal | IPv4address | reg-name
// IP-literal		=	'[' ( IPv6address | IPvFuture ) ']'
// IPvFuture		=	'v' 1*HEXDIG '.' 1*( unreserved | sub-delims | ':' )
// IPv6address		=									6(	h16 ':' )	ls32
// 					|							':'':'	5(	h16 ':' )	ls32
// 					|	[ 				h16 ] 	':'':'	4(	h16 ':' )	ls32
// 					|	[ *1( h16 ':' )	h16 ]	':'':'	3(	h16 ':' )	ls32
// 					|	[ *2( h16 ':' )	h16 ]	':'':'	2(	h16 ':' )	ls32
// 					|	[ *3( h16 ':' )	h16 ]	':'':'		h16 ':'		ls32
// 					|	[ *4( h16 ':' )	h16 ]	':'':'					ls32
// 					|	[ *5( h16 ':' )	h16 ]	':'':'					h16
// 					|	[ *6( h16 ':' )	h16 ]	':'':'
// ls32				=	( h16 ':' h16 ) | IPv4address
// h16				=	1*4HEXDIG
// IPv4address		=	dec-octet '.' dec-octet '.' dec-octet '.' dec-octet
// dec-octet		=	DIGIT				; 0-9
// 					|	%x31-39 DIGIT		; 10-99
// 					|	'1' 2DIGIT			; 100-199
// 					|	'2' %x30-34 DIGIT	; 200-249
// 					|	'25' %x30-35		; 250-255
// reg-name			=	*( unreserved | pct-encoded | sub-delims )
// port				=	*DIGIT
// path-abempty		=	*( '/' segment )
// path-absolute	=	'/' [ segment-nz *( '/' segment ) ]
// segment-nz		=	1*pchar
// path-rootless	=	segment-nz *( '/' segment )
// path-empty		=	0<pchar> ; match NOTHING
//
// authority-form	=	uri-host ':' port
// uri-host			=	host
//
// asterisk-form	=	'*'
//
// HTTP-version		=	HTTP-name '/' DIGIT '.' DIGIT
// HTTP-name		=	'H''T''T''P'
//
// status-line		=	HTTP-version ' ' status-code ' ' [ reason-phrase ]
// status-code		=	3DIGIT
// reason-phrase	=	1*( '\t' | ' ' | VCHAR | obs-text )
// obs-text			=	%x80-FF
// VCHAR			=	%x21-7E
//
// field-line		=	field-name ':' OWS field-value OWS
// field-name		=	token
// field-value		=	*field-content
// field-content	=	field-vchar
//						[ 1*( ' ' | '\t' | field-vchar ) field-vchar ]
// field-vchar		=	VCHAR | obs-text
// OWS				=	*( ' ' | '\t' )
// RWS				=	1*( ' ' | '\t' )
// obs-fold			=	OWS '\r''\n' RWS	; something something message/http
//
// message-body		=	*OCTET ; any amount of any characters :fire:
