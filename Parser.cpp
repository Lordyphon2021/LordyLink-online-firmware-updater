#include "Parser.h"
#include "boost/asio.hpp"

using namespace std;
using namespace boost::asio;




bool CParser::writeFileToContainer()
{
    string temp_str;
    
    hex_file.open("firmware.txt");

    if(hex_file){
        //file in buffer einlesen
        while (getline(hex_file, temp_str)) {
            record_collection.push_back(temp_str);
            temp_str.clear();
        }

        hex_file.close();
        return true;
    }
    return false;

}

bool CParser::GetPageContent() {
    

    ofstream firmware;
    firmware.open("Firmware.txt", ios_base::out);

    try {
        boost::asio::io_service io_service;

        ip::tcp::resolver resolver(io_service);
        ip::tcp::resolver::query query("66.96.162.138", "80");
        ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        ip::tcp::resolver::iterator end;
        // Try each endpoint until we successfully establish a connection.
        ip::tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end) {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }
        if (error) {
           
            throw boost::system::system_error(error);
            return false;
        }
        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << "http://stefandeisenberger86881.domain.com/firmware/lordyphon_proto.txt" << " HTTP/1.0\r\n";
        request_stream << "Host: " << "66.96.162.138" << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";
        // Send the request.
        write(socket, request);
        // Read the response status line.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");
        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::clog << "Invalid response\n";
            return false;
        }
        if (status_code != 200) {
            std::clog << "Response returned with status code " << status_code << "\n";
            return false;
        }
       
        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;

        while (std::getline(response_stream, header) && header != "\r")
            std::clog << header << "\n";




        // Write whatever content we already have to output.
        if (response.size() > 0) {
            firmware << &response;
        }
        else
            return false;
        // Read until EOF, writing data to output as we go.
        while (true) {
            size_t n = read(socket, response, boost::asio::transfer_at_least(1), error);

            if (!error)
            {
                if (n)
                    firmware << &response;


            }

            if (error == boost::asio::error::eof) {
               
                return true;
                break;
            }

            if (error) {
               
                throw boost::system::system_error(error);
            }
        }
    }
    catch (std::exception& e) {
        std::clog << "Exception: " << e.what() << endl;
    }

    firmware.close();
    return false;

   

}


bool CParser::parseFile()
{
    return false;
}

bool CParser::sendParsedToUsb()
{
    return false;
}
