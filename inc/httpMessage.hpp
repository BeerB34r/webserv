/* ************************************************************************** */
/*                                                                            */
/*                                                            ::::::::        */
/*   httpMessage.hpp                                         :+:    :+:       */
/*                                                          +:+               */
/*   By: mde-beer <mde-beer@student.codam.nl>              +#+                */
/*                                                        +#+                 */
/*   Created: 2026/02/11 14:30:55 by mde-beer            #+#    #+#           */
/*   Updated: 2026/02/11 16:57:10 by mde-beer            ########   odam.nl   */
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

#ifndef HTTPMESSAGE_HPP
# define HTTPMESSAGE_HPP

// [NOTE] taken from RFC-editor.org
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

class	HttpMessage {
public:
	HttpMessage();
	HttpMessage(const HttpMessage&);
	HttpMessage&	operator =(const HttpMessage&);
	virtual	~HttpMessage();

private:
	
protected:
};

#endif // HTTPMESSAGE_HPP
