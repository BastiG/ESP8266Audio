/*
 spiram-fast - Fast, hardcoded interface for SPI-based RAMs, allowing DIO
               mode to be used and speeding up individual SPI operations
               significantly.

 Copyright (c) 2020 Earle F. Philhower, III   All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#ifndef spiram_fast_h
#define spiram_fast_h

#include <Arduino.h>
#include <SPI.h>

class ESP8266SPIRAM {
    private:
        uint8_t _cs;
        SPIClass _spi;
        SPISettings _settings;

        // The standard VSPI bus pins are used
        static constexpr uint8_t miso = 19;
        static constexpr uint8_t mosi = 23;
        static constexpr uint8_t sck = 18;

    public:
        ESP8266SPIRAM() :
            _spi(VSPI),
            _settings(20000000, MSBFIRST, SPI_MODE0)
        {
            /* noop */
        }
        ~ESP8266SPIRAM()
        {
            end();
        }
        inline void beginTransation() {
            _spi.beginTransaction(_settings);
            digitalWrite(_cs, LOW);
        }
        inline void endTransaction() {
            digitalWrite(_cs, HIGH);
            _spi.endTransaction();
        }
        void readBytes(uint32_t addr, void *destV, int count)
        {
            uint8_t *dest = (uint8_t *)destV;
            beginTransation();
            uint32_t cmd = (0x03 << 24) | (addr & ((1<<24)-1));
            _spi.transfer((uint8_t)(cmd >> 24));
            _spi.transfer((uint8_t)(cmd >> 16));
            _spi.transfer((uint8_t)(cmd >> 8));
            _spi.transfer((uint8_t)(cmd >> 0));
            while (count--) {
                *(dest++) = _spi.transfer((uint8_t)0);
            }
            endTransaction();
        }

        void writeBytes(uint32_t addr, const void *srcV, int count)
        {
            const uint8_t *src = (const uint8_t *)srcV;
            beginTransation();
            uint32_t cmd = (0x02 << 24) | (addr & ((1<<24)-1));
            _spi.transfer((uint8_t)(cmd >> 24));
            _spi.transfer((uint8_t)(cmd >> 16));
            _spi.transfer((uint8_t)(cmd >> 8));
            _spi.transfer((uint8_t)(cmd >> 0));
            while (count--) {
                _spi.transfer((uint8_t)*(src++));
            }
            endTransaction();
        }

        void begin(int freqMHz, int cs_pin)
        {
            _cs = cs_pin;
            _settings._clock = freqMHz * 1000000;

            _spi.begin(sck, miso, mosi, _cs);

            pinMode(_cs, OUTPUT);
            digitalWrite(_cs, HIGH);
            delay(50);
            digitalWrite(_cs, LOW);
            delay(50);
            digitalWrite(_cs, HIGH);

            // Enable streaming read/write mode
            beginTransation();
            _spi.transfer((uint8_t) 0x01);
            _spi.transfer((uint8_t) 0x40);
            endTransaction();
        }

        void end()
        {
            pinMode(_cs, INPUT);
            pinMode(miso, INPUT);
            pinMode(mosi, INPUT);
            pinMode(sck, INPUT);
            _spi.end();
        }

};

#endif