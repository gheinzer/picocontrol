let lastMovementTime = 0;
let movementX = 0;
let movementY = 0;
let movementWheel = 0;
let pointerLockActive = false;
let activeKeyboardKeys = new Set();
let activeMouseKeys = new Set();

let bluetoothActive = false;
let usbActive = false;

function toggleBluetooth() {
    bluetoothActive = !bluetoothActive;
    quickAction('set-bluetooth', bluetoothActive ? 'on' : 'off');
    if(bluetoothActive)
        document.getElementById('bluetooth-toggle').classList.add('checked');
    else
        document.getElementById('bluetooth-toggle').classList.remove('checked');
}

function toggleUSB() {
    usbActive = !usbActive;
    quickAction('set-usb', usbActive ? 'on' : 'off');
    if(usbActive)
        document.getElementById('usb-toggle').classList.add('checked');
    else
        document.getElementById('usb-toggle').classList.remove('checked');
}

function openWebsocket() {
    ws = new WebSocket("ws:/" + location.hostname + ":81");
    ws.onopen = toggleBluetooth;
}

function quickAction(action, value) {
    console.log('actions.' + action + '=' + value);
    ws.send('actions.' + action + '=' + value);
}

function keydown(keyCode)  {
    console.log('key.down=' + keyCode);
    ws.send('key.down=' + keyCode);

    activeKeyboardKeys.add(keyCode);
}

function keyup(keyCode)  {
    console.log('key.up=' + keyCode);
    ws.send('key.up=' + keyCode);

    activeKeyboardKeys.delete(keyCode);
}

function mousemoveHandler(event) {
    mouseAction(event, event.movementX, event.movementY, 0);
}

function mousewheelHandler(event) {
    mouseAction(event, 0, 0, event.deltaY);
}

function mousedown(buttonCode) {
    ws.send('mouse.down=' + buttonCode);
    activeMouseKeys.add(buttonCode);
} 

function mouseup(buttonCode) {
    ws.send('mouse.up=' + buttonCode);
    activeMouseKeys.delete(buttonCode);
}

function keydownHandler(event) {
    event.preventDefault();
    event.stopPropagation();
    keydown(event.key);
}
function keyupHandler(event) {
    event.preventDefault();
    event.stopPropagation();
    keyup(event.key);
}
function mousedownHandler(event) {
    event.preventDefault();
    event.stopPropagation();
    mousedown(event.button);
}
function mouseupHandler(event) {
    event.preventDefault();
    event.stopPropagation();
    mouseup(event.button);
}

function mouseAction(event, x, y, wheel) {
    event.preventDefault();
    event.stopPropagation();

    movementX += x;
    movementY += y;
    movementWheel += wheel;

    if (Date.now() - lastMovementTime > 10) {
        lastMovementTime = Date.now();
        ws.send('mouse.move=' + movementX + ',' + movementY + ',' + movementWheel);

        movementX = 0;
        movementY = 0;
        movementWheel = 0;
    }
}

document.addEventListener('pointerlockchange', () => {
    if (document.pointerLockElement === document.body) {
        document.addEventListener('keydown', keydownHandler);
        document.addEventListener('keyup', keyupHandler);
        document.addEventListener('mousemove', mousemoveHandler);
        document.addEventListener('mousedown', mousedownHandler);
        document.addEventListener('mouseup', mouseupHandler);
        document.addEventListener('wheel', mousewheelHandler);

        document.body.style.filter = 'brightness(30%) blur(3px)';
        pointerLockActive = true;
    } else {
        document.removeEventListener('keydown', keydownHandler);
        document.removeEventListener('keyup', keyupHandler);
        document.removeEventListener('mousemove', mousemoveHandler);
        document.removeEventListener('mousedown', mousedownHandler);
        document.removeEventListener('mouseup', mouseupHandler);
        document.removeEventListener('wheel', mousewheelHandler);

        // Release all still-pressed keys
        activeKeyboardKeys.forEach(keyup);
        activeMouseKeys.forEach(mouseup);

        document.body.style.filter = '';
        pointerLockActive = false;
    }
});

document.addEventListener('keydown', async (event) => {
    if(event.ctrlKey && event.shiftKey && event.altKey) {
        if(pointerLockActive) {
            document.exitPointerLock();
        } else {
            document.body.focus();
            document.body.requestPointerLock();
        }
    }
})

var versionRequest = new XMLHttpRequest();
versionRequest.open("GET", "version.txt", false); // false for synchronous request
versionRequest.send(null);
document.getElementById('footer-version').innerText = versionRequest.responseText;

openWebsocket();