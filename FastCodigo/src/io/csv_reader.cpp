#include "io/csv_reader.h" // Assuming viab::Dia is defined here or in an included header
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>   // Make sure vector is included
#include <string>   // Make sure string is included
#include <iostream> // For potential error logging (e.g., std::cerr)

// User-provided definition of viab::Dia (or ensure it's correctly included from "io/csv_reader.h")
// #pragma once // This pragma is for header files, not typically in .cpp files directly
// namespace viab {
// struct Dia {
//     std::string data_str;
//     int mes;
//     double tmax;
//     double tmin;
//     // Implicitly available:
//     // Dia() = default;
//     // Dia(const std::string& ds, int m, double t_max, double t_min) : data_str(ds), mes(m), tmax(t_max), tmin(t_min) {} // if you had a constructor
// };
// } // namespace viab
// Note: The actual struct definition should be in "io/csv_reader.h" or a related header.

namespace io {
std::vector<viab::Dia> ler_dados(const std::string& file){
    // Open the file for reading
    std::ifstream ifs(file);
    // Check if the file was opened successfully
    if(!ifs) {
        // If not, throw a runtime error with the filename
        throw std::runtime_error("Could not open file: " + file);
    }

    std::vector<viab::Dia> v; // Vector to store Dia objects
    std::string l;            // String to store each line

    // Read and discard the header line
    if (!std::getline(ifs,l)) {
        // Handle empty or unreadable file (e.g., only header or less)
        // std::cerr << "Warning: File is empty or header is missing: " << file << std::endl;
        return v; // Return empty vector
    }

    // Loop through the rest of the lines in the file
    while(std::getline(ifs,l)){
        // Use a stringstream to parse each line
        std::stringstream ss(l);
        std::string dt, tmx, tmn; // Strings to store parts of the line

        // Extract the date string (dt)
        if(!std::getline(ss,dt,';')) {
            // If reading dt fails, skip to the next line
            // std::cerr << "Warning: Skipping malformed line (missing date): " << l << std::endl;
            continue;
        }
        // Extract tmx and tmn strings
        if(!std::getline(ss,tmx,';') || !std::getline(ss,tmn,';')){
            // If reading tmx or tmn fails, skip to the next line
            // std::cerr << "Warning: Skipping malformed line (missing tmax or tmin): " << l << std::endl;
            continue;
        }

        try {
            // Convert the relevant part of the date string (dt) to an integer for the month
            // dt.substr(3,2) assumes format like "dd/MM/yyyy" or "dd-MM-yyyy"
            // Ensure dt is long enough and has the expected format before substr
            int mes_val = 0;
            if (dt.length() >= 5) { // Basic check for "dd/MM" or "dd-MM"
                 mes_val = std::stoi(dt.substr(3,2)); // Extracts month, e.g., "05" from "01/05/2023"
            } else {
                // Handle error: date string too short or malformed
                // std::cerr << "Warning: Malformed or too short date string, skipping: " << dt << " on line: " << l << std::endl;
                continue;
            }

            // Convert tmx and tmn to doubles
            double tmx_val = std::stod(tmx);
            double tmn_val = std::stod(tmn);

            // Explicitly construct the viab::Dia object using aggregate initialization
            // This now matches the user-provided struct definition which has 4 members.
            v.push_back(viab::Dia{dt, mes_val, tmx_val, tmn_val});

            // Alternative way if you had a constructor or prefer named construction:
            // viab::Dia dia_obj;
            // dia_obj.data_str = dt;
            // dia_obj.mes = mes_val;
            // dia_obj.tmax = tmx_val;
            // dia_obj.tmin = tmn_val;
            // v.push_back(dia_obj);

        } catch (const std::invalid_argument& ia) {
            // Handle cases where std::stoi or std::stod fails (e.g., non-numeric data)
            // std::cerr << "Invalid argument for conversion: " << ia.what() << " on line: " << l << std::endl;
            // Decide whether to skip the line, use default values, or rethrow
            continue;
        } catch (const std::out_of_range& oor) {
            // Handle cases where the converted value is out of range for the type
            // std::cerr << "Out of range for conversion: " << oor.what() << " on line: " << l << std::endl;
            continue;
        }
    }
    return v; // Return the vector of Dia objects
}
} // namespace io
