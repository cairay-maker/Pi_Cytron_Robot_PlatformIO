#include "RPi_Interface.h"

RPi_Interface::RPi_Interface() {}

void RPi_Interface::begin() {
    // Hardware UART1 is now configured on GP4(TX) and GP5(RX)
    // DUMMY MODE: Hardware calls are commented out to test for lockups
    
    // Serial1.setTX(4);
    // Serial1.setRX(5);
    // pinMode(5, INPUT_PULLUP);
    // Serial1.begin(115200);
    
    Serial.println("RPi Interface: DUMMY MODE (Hardware UART Disabled to test IRQ conflicts)");
}

void RPi_Interface::update() {
    // DUMMY MODE: Hardware read loop is bypassed to test for lockups
    /*
    int bytesRead = 0;
    // Cap reading at 64 bytes per loop to prevent infinite noise lockups
    while (Serial1.available() > 0 && bytesRead < 64) {
        char inChar = Serial1.read();
        bytesRead++;
        
        if (inChar == '\n' || inChar == '\r') {
            if (_bufIndex > 0) {
                _buffer[_bufIndex] = '\0'; // Null-terminate string
                parsePacket();
                _bufIndex = 0; // Reset for next packet
            }
        } else if (_bufIndex < MAX_PACKET_LEN - 1) {
            _buffer[_bufIndex++] = inChar;
        }
    }
    */
}

void RPi_Interface::parsePacket() {
    // Expected format: V:<error>,<greenCode>
    // Example: "V:150,0" or "V:-45,2"
    if (_buffer[0] == 'V' && _buffer[1] == ':') {
        char* commaIndex = strchr(_buffer, ',');
        
        if (commaIndex != NULL) {
            *commaIndex = '\0'; // Split the string at the comma
            
            // Convert strings to integers
            _currentError = atoi(&_buffer[2]); // Start reading after "V:"
            _currentGreenCode = atoi(commaIndex + 1); // Start reading after comma
            
            _isFresh = true;
        }
    }
}

int RPi_Interface::getPixelError() {
    _isFresh = false; // Mark data as read
    return _currentError;
}

int RPi_Interface::getGreenCode() {
    return _currentGreenCode;
}

bool RPi_Interface::isDataFresh() {
    return _isFresh;
}

void RPi_Interface::setMockVision(int error, int greenCode) {
    _currentError = error;
    _currentGreenCode = greenCode;
    _isFresh = true;
    
    Serial.print(">>> Mock Vision Set -> Error: ");
    Serial.print(_currentError);
    Serial.print(" | Green Code: ");
    Serial.println(_currentGreenCode);
}