#include <Arduino.h>
#include <WiFi.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <WebFS.h>
#include <WebSocketsServer.h>
#include <RP2040.h>
#include <DNSServer.h>
#include <KeyboardBLE.h>
#include <MouseBLE.h>

WiFiServer http(80);
WebSocketsServer websocket(81);
DNSServer dns;
IPAddress apIP(10, 1, 1, 1);

void keyboard_press(uint8_t key) {
    Keyboard.press(key);
    KeyboardBLE.press(key);
}

void keyboard_release(uint8_t key) {
    Keyboard.release(key);
    KeyboardBLE.release(key);
}

void keyboard_print(arduino::String value) {
    Keyboard.print(value);
    KeyboardBLE.print(value);
}

void mouse_press(uint8_t key) {
    Mouse.press(key);
    MouseBLE.press(key);
}

void mouse_release(uint8_t key) {
    Mouse.release(key);
    MouseBLE.release(key);
}

void mouse_move(int x, int y, signed char wheel) {
    Mouse.move(x, y, wheel);
    MouseBLE.move(x, y, wheel);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload_chararray, size_t length) {
    if(type == WStype_TEXT) {
        arduino::String payloadString = (char*)payload_chararray;
        unsigned int delimiter_location = payloadString.indexOf("=");
        arduino::String request = payloadString.substring(0, delimiter_location);
        arduino::String value = payloadString.substring(delimiter_location + 1, -1);

        if(request == "key.down" || request == "key.up") {
            uint8_t key_code;
            bool ignore = false;
            
            if(value.length() == 1) key_code = value.charAt(0);

            // Keyboard modifiers
            else if(value == "Control") key_code = KEY_LEFT_CTRL;
            else if(value == "Shift") key_code = KEY_LEFT_SHIFT;
            else if(value == "Alt") key_code = KEY_LEFT_ALT;
            else if(value == "AltGraph") key_code = KEY_RIGHT_ALT;
            else if(value == "Meta") key_code = KEY_LEFT_GUI;

            // Special keys within the alphanumeric cluster
            else if(value == "Tab") key_code = KEY_TAB;
            else if(value == "CapsLock") key_code = KEY_CAPS_LOCK;
            else if(value == "Backspace") key_code = KEY_BACKSPACE;
            else if(value == "Enter") key_code = KEY_RETURN;
            else if(value == "ContextMenu") key_code = KEY_MENU;

            // Navigation cluster
            else if(value == "Insert") key_code = KEY_INSERT;
            else if(value == "Delete") key_code = KEY_DELETE;
            else if(value == "Home") key_code = KEY_HOME;
            else if(value == "End") key_code = KEY_END;
            else if(value == "PageDown") key_code = KEY_PAGE_DOWN;
            else if(value == "PageUp") key_code = KEY_PAGE_UP;
            else if(value == "ArrowUp") key_code = KEY_UP_ARROW;
            else if(value == "ArrowDown") key_code = KEY_DOWN_ARROW;
            else if(value == "ArrowLeft") key_code = KEY_LEFT_ARROW;
            else if(value == "ArrowRight") key_code = KEY_RIGHT_ARROW;

            // Numeric keypad
            else if(value == "NumLock") key_code = KEY_NUM_LOCK;
            else if(value == "Divide") key_code = KEY_KP_SLASH;
            else if(value == "Multiply") key_code = KEY_KP_ASTERISK;
            else if(value == "Subtract") key_code = KEY_KP_MINUS;
            else if(value == "Add") key_code = KEY_KP_PLUS;
            // Digits 0 - 9 are handled normally
            else if(value == "Separator" || value == "Decimal") key_code = KEY_KP_DOT;
            else if(value == "NumLock") key_code = KEY_NUM_LOCK;

            // Escape and function keys
            else if(value == "Escape") key_code = KEY_ESC;
            else if(value == "F1") key_code = KEY_F1;
            else if(value == "F2") key_code = KEY_F2;
            else if(value == "F3") key_code = KEY_F3;
            else if(value == "F4") key_code = KEY_F4;
            else if(value == "F5") key_code = KEY_F5;
            else if(value == "F6") key_code = KEY_F6;
            else if(value == "F7") key_code = KEY_F7;
            else if(value == "F8") key_code = KEY_F8;
            else if(value == "F9") key_code = KEY_F9;
            else if(value == "F10") key_code = KEY_F10;
            else if(value == "F11") key_code = KEY_F11;
            else if(value == "F12") key_code = KEY_F12;
            else if(value == "F13") key_code = KEY_F13;
            else if(value == "F14") key_code = KEY_F14;
            else if(value == "F15") key_code = KEY_F15;
            else if(value == "F16") key_code = KEY_F16;
            else if(value == "F17") key_code = KEY_F17;
            else if(value == "F18") key_code = KEY_F18;
            else if(value == "F19") key_code = KEY_F19;
            else if(value == "F20") key_code = KEY_F20;
            else if(value == "F21") key_code = KEY_F21;
            else if(value == "F22") key_code = KEY_F22;
            else if(value == "F23") key_code = KEY_F23;
            else if(value == "F24") key_code = KEY_F24;

            // Function control keys
            else if(value == "PrintScreen") key_code = KEY_PRINT_SCREEN;
            else if(value == "ScrollLock") key_code = KEY_SCROLL_LOCK;
            else if(value == "Pause") key_code = KEY_PAUSE;
            
            // Ignore everything else
            else ignore = true;
            
            if(request == "key.down" && !ignore) {
                keyboard_press(key_code);
            } else if(request == "key.up" && !ignore) {
                keyboard_release(key_code);
            }
        } else if(request == "mouse.down" || request == "mouse.up") {
            unsigned int button = value.toInt();
            unsigned int mouse_button = MOUSE_LEFT;
            if(button == 0) mouse_button = MOUSE_LEFT;
            else if(button == 1) mouse_button = MOUSE_MIDDLE;
            else if(button == 2) mouse_button = MOUSE_RIGHT;

            if(request == "mouse.down") {
                mouse_press(mouse_button);
            } else {
                mouse_release(mouse_button);
            }
        } else if(request == "mouse.move") {
            unsigned int delimiter_pos = value.indexOf(",");
            unsigned int last_delimiter_pos = value.lastIndexOf(",");
            signed int movement_x = value.substring(0, delimiter_pos).toInt();
            signed int movement_y = value.substring(delimiter_pos + 1, last_delimiter_pos).toInt();
            signed int movement_wheel = value.substring(last_delimiter_pos + 1, -1).toInt();
            mouse_move(movement_x, movement_y, movement_wheel);
        } else if(request == "actions.lock") {
            keyboard_press(KEY_LEFT_GUI);
            keyboard_press('l');
            delay(5);
            keyboard_release(KEY_LEFT_GUI);
            keyboard_release('l');
        } else if(request == "actions.alttab") {
            keyboard_press(KEY_LEFT_ALT);
            keyboard_press(KEY_TAB);
            delay(5);
            keyboard_release(KEY_LEFT_ALT);
            keyboard_release(KEY_TAB);
        } else if(request == "actions.altf4") {
            keyboard_press(KEY_LEFT_ALT);
            keyboard_press(KEY_F4);
            delay(5);
            keyboard_release(KEY_LEFT_ALT);
            keyboard_release(KEY_F4);
        } else if(request == "actions.exec") {
            keyboard_press(KEY_LEFT_GUI);
            keyboard_press('r');
            delay(5);
            keyboard_release(KEY_LEFT_GUI);
            keyboard_release('r');
            keyboard_print(value);
        }
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    // Initialize network connection over WiFi
    WiFi.setHostname("picocontrol");
    arduino::String ssid = "picocontrol_" + arduino::String(rp2040.getChipID()).substring(0, 6);
    
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.beginAP(ssid.c_str());

    // Initialize the USB HID interface
    Keyboard.begin(KeyboardLayout_de_DE);
    KeyboardBLE.begin(ssid.c_str(), ssid.c_str(), KeyboardLayout_de_DE);
    Mouse.begin();
    MouseBLE.begin(ssid.c_str(), ssid.c_str());

    // Initialize everything web-server-related
    WebFS.init();
    http.begin();
    websocket.begin();
    websocket.onEvent(webSocketEvent);
    dns.start(53, "*", WiFi.softAPIP());
    dns.setErrorReplyCode(DNSReplyCode::NoError);
}

void loop() {
    while(http.hasClient()) {
        WiFiClient client = http.accept();

        arduino::String requestLine = client.readStringUntil('\n');
        arduino::String host = "abc.internal";
        while(1) {
            arduino::String req = client.readStringUntil('\n');
            if(req.startsWith("Host: ")) {
                host = req.substring(req.indexOf(' ') + 1, -1);
                host.replace("\r", "");
            } else if(req == "\r") break;
        }

        if(host == "picocontrol.internal") {
            arduino::String path = requestLine.substring(requestLine.indexOf(' ') + 2, requestLine.lastIndexOf(' '));
            if(path.isEmpty()) path = "index.html";
            
            // send a standard http response header
            WebFS_Handle* file = WebFS.getFile(std::string(path.c_str()));
            if(file == 0) {
                client.println("HTTP/1.1 404 Not Found");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");  // the connection will be closed after completion of the response
                client.println();
                client.print("File not found: ");
                client.println(path);
            } else {
                client.println("HTTP/1.1 200 OK");
                client.print("Content-Type: ");
                client.println(file->mime_type.c_str());
                client.println("Connection: close");  // the connection will be closed after completion of the response
                client.println();

                client.write(file->content, file->size);
            }   
        } else {
            client.println("HTTP/1.1 302 Found");
            client.println("Content-Type: text/plain");
            client.println("Location: http://picocontrol.internal/");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println();
            client.print("You are begin redirected...");
        }

        client.println();
        client.flush();

        delay(1); // Give the browser some time to receive

        client.stop();
    }

    websocket.loop();
    dns.processNextRequest();
}