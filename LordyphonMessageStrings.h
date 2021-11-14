#pragma once

#include <QByteArray>

struct LordyphonCall {
	const QByteArray hand_shake_tx_phrase = "!c++ is great!   "; //HANDSHAKE CALL
	const QByteArray update_tx_phrase = "update";	//UPDATE MODE CALL
	const QByteArray burn_flash = "w";
	const QByteArray get_checksum_status = "?";
	const QByteArray dump_request = "r";
	const QByteArray request_checksum = "s";
	const QByteArray transfer_request = "R";
	const QByteArray begin_tansfer = "/";
	const QByteArray burn_eeprom = "ß";
	const QByteArray abort = ".";
	const QByteArray abort2 = ";";
};


struct LordyphonResponse {
	const QByteArray hand_shake_rx_phrase = "YES!";  //HANDSHAKE RESPONSE
	const QByteArray update_rx_phrase_y = "sure";	 //UPDATE MODE RESPONSE
	const QByteArray update_rx_phrase_n = "nope";	 //UPDATE MODE RESPONSE
	const QByteArray is_checksum_error = "er";		 //checksum error
	const QByteArray checksum_ok = "ok";			 //checksum ok
	const QByteArray play_mode_is_off = "doit";		 //data rx confirmed
	const QByteArray play_mode_is_on = "DONT";		 //data rx declined
};
