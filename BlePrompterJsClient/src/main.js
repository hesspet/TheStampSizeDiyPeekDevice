const BLE_PROMPTER_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_PROMPTER_RECEIVE_CHARACTERISTIC_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_PROMPTER_TRANSMIT_CHARACTERISTIC_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

const connectButton = document.querySelector("#connectButton");
const showAllDevicesButton = document.querySelector("#showAllDevicesButton");
const disconnectButton = document.querySelector("#disconnectButton");
const clearButton = document.querySelector("#clearButton");
const helpButton = document.querySelector("#helpButton");
const invertToggle = document.querySelector("#invertToggle");
const connectionState = document.querySelector("#connectionState");
const modeTabs = [...document.querySelectorAll(".mode-tab")];
const modePanels = [...document.querySelectorAll(".mode-panel")];
const commandButtons = [...document.querySelectorAll(".command-button")];
const suitButtons = [...document.querySelectorAll(".suit-button")];
const rankButtons = [...document.querySelectorAll(".rank-button")];
const symbolForm = document.querySelector("#symbolForm");
const symbolInput = document.querySelector("#symbolInput");
const sendSymbolButton = document.querySelector("#sendSymbolButton");
const lastCommandOutput = document.querySelector("#lastCommandOutput");
const lastResponseOutput = document.querySelector("#lastResponseOutput");
const messageLog = document.querySelector("#messageLog");

let bluetoothDevice = null;
let bluetoothServer = null;
let bluetoothReceiveCharacteristic = null;
let bluetoothTransmitCharacteristic = null;
let selectedSuitCode = "H";
let isConnected = false;

function addLogEntry(messageText) {
  const listItem = document.createElement("li");
  const timestamp = new Intl.DateTimeFormat("de-DE", {
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  }).format(new Date());

  listItem.textContent = `${timestamp} ${messageText}`;
  messageLog.prepend(listItem);

  while (messageLog.children.length > 8) {
    messageLog.lastElementChild.remove();
  }
}

function setConnectionState(connected, statusText) {
  isConnected = connected;
  connectionState.classList.toggle("connected", connected);
  connectionState.querySelector("span:last-child").textContent = statusText;

  connectButton.disabled = connected;
  showAllDevicesButton.disabled = connected;
  disconnectButton.disabled = !connected;
  clearButton.disabled = !connected;
  helpButton.disabled = !connected;
  invertToggle.disabled = !connected;
  symbolInput.disabled = !connected;
  sendSymbolButton.disabled = !connected;

  for (const button of commandButtons) {
    button.disabled = !connected;
  }
}

function resetBluetoothState(statusText = "Nicht verbunden") {
  bluetoothServer = null;
  bluetoothReceiveCharacteristic = null;
  bluetoothTransmitCharacteristic = null;
  setConnectionState(false, statusText);
}

function getBluetoothUnavailableReason() {
  if (!window.isSecureContext) {
    return "Web Bluetooth braucht HTTPS oder localhost.";
  }

  if (!navigator.bluetooth) {
    return "Dieser Browser unterstützt Web Bluetooth nicht.";
  }

  return "";
}

function getFilteredDeviceRequestOptions() {
  return {
    filters: [
      { namePrefix: "BlePrompter" },
      { services: [BLE_PROMPTER_SERVICE_UUID] },
    ],
    optionalServices: [BLE_PROMPTER_SERVICE_UUID],
  };
}

function getAllDevicesRequestOptions() {
  return {
    acceptAllDevices: true,
    optionalServices: [BLE_PROMPTER_SERVICE_UUID],
  };
}

async function connectBlePrompter(showAllDevices = false) {
  const unavailableReason = getBluetoothUnavailableReason();
  if (unavailableReason) {
    lastResponseOutput.value = unavailableReason;
    addLogEntry(unavailableReason);
    return;
  }

  connectButton.disabled = true;
  showAllDevicesButton.disabled = true;
  connectionState.querySelector("span:last-child").textContent = "Gerät auswählen";

  try {
    bluetoothDevice = await navigator.bluetooth.requestDevice(
      showAllDevices ? getAllDevicesRequestOptions() : getFilteredDeviceRequestOptions()
    );

    bluetoothDevice.addEventListener("gattserverdisconnected", handleBluetoothDisconnected);
    connectionState.querySelector("span:last-child").textContent = "Verbinde";

    bluetoothServer = await bluetoothDevice.gatt.connect();
    const bluetoothService = await bluetoothServer.getPrimaryService(BLE_PROMPTER_SERVICE_UUID);

    bluetoothReceiveCharacteristic = await bluetoothService.getCharacteristic(
      BLE_PROMPTER_RECEIVE_CHARACTERISTIC_UUID
    );
    bluetoothTransmitCharacteristic = await bluetoothService.getCharacteristic(
      BLE_PROMPTER_TRANSMIT_CHARACTERISTIC_UUID
    );

    await startNotifications();
    setConnectionState(true, "Verbunden");
    addLogEntry(`Verbunden mit ${bluetoothDevice.name || "BlePrompter"}`);
  } catch (error) {
    resetBluetoothState("Nicht verbunden");
    lastResponseOutput.value = getDisplayErrorMessage(error);
    addLogEntry(getDisplayErrorMessage(error));
  }
}

function disconnectBlePrompter() {
  if (bluetoothDevice?.gatt?.connected) {
    bluetoothDevice.gatt.disconnect();
    return;
  }

  resetBluetoothState();
}

function handleBluetoothDisconnected() {
  resetBluetoothState("Getrennt");
  addLogEntry("Verbindung getrennt");
}

async function startNotifications() {
  if (!bluetoothTransmitCharacteristic) {
    return;
  }

  await bluetoothTransmitCharacteristic.startNotifications();
  bluetoothTransmitCharacteristic.addEventListener("characteristicvaluechanged", (event) => {
    const responseText = new TextDecoder().decode(event.target.value);
    lastResponseOutput.value = responseText;
    addLogEntry(`Antwort: ${responseText}`);
  });
}

async function sendBlePrompterCommand(commandText) {
  if (!bluetoothReceiveCharacteristic || !isConnected) {
    const messageText = "BlePrompter ist nicht verbunden.";
    lastResponseOutput.value = messageText;
    addLogEntry(messageText);
    return;
  }

  const commandBytes = new TextEncoder().encode(commandText);
  lastCommandOutput.value = commandText;

  try {
    if (typeof bluetoothReceiveCharacteristic.writeValueWithoutResponse === "function") {
      await bluetoothReceiveCharacteristic.writeValueWithoutResponse(commandBytes);
    } else {
      await bluetoothReceiveCharacteristic.writeValueWithResponse(commandBytes);
    }

    addLogEntry(`Gesendet: ${commandText}`);
  } catch (error) {
    const errorMessage = getDisplayErrorMessage(error);
    lastResponseOutput.value = errorMessage;
    addLogEntry(errorMessage);
  }
}

function getDisplayErrorMessage(error) {
  if (error?.name === "NotFoundError") {
    return "Kein Gerät ausgewählt.";
  }

  if (error?.name === "SecurityError") {
    return "Der Browser blockiert Web Bluetooth in diesem Kontext.";
  }

  if (error?.name === "NetworkError") {
    return "Die BLE-Verbindung ist fehlgeschlagen.";
  }

  if (error?.name === "NotSupportedError") {
    return "Diese BLE-Aktion wird vom Browser nicht unterstützt.";
  }

  return "Aktion fehlgeschlagen.";
}

function selectMode(modeName) {
  for (const tab of modeTabs) {
    const isActive = tab.dataset.mode === modeName;
    tab.classList.toggle("active", isActive);
    tab.setAttribute("aria-pressed", String(isActive));
  }

  for (const panel of modePanels) {
    panel.classList.toggle("active", panel.dataset.panel === modeName);
  }
}

function selectSuit(suitCode) {
  selectedSuitCode = suitCode;

  for (const button of suitButtons) {
    const isActive = button.dataset.suitCode === suitCode;
    button.classList.toggle("active", isActive);
    button.setAttribute("aria-pressed", String(isActive));
  }
}

connectButton.addEventListener("click", () => connectBlePrompter(false));
showAllDevicesButton.addEventListener("click", () => connectBlePrompter(true));
disconnectButton.addEventListener("click", disconnectBlePrompter);
clearButton.addEventListener("click", () => sendBlePrompterCommand("CL"));
helpButton.addEventListener("click", () => sendBlePrompterCommand("H"));

invertToggle.addEventListener("change", () => {
  sendBlePrompterCommand(invertToggle.checked ? "I1" : "I0");
});

for (const tab of modeTabs) {
  tab.addEventListener("click", () => selectMode(tab.dataset.mode));
}

for (const button of commandButtons) {
  button.addEventListener("click", () => {
    if (button.dataset.command) {
      sendBlePrompterCommand(button.dataset.command);
    }
  });
}

for (const button of suitButtons) {
  button.addEventListener("click", () => selectSuit(button.dataset.suitCode));
}

for (const button of rankButtons) {
  button.addEventListener("click", () => {
    sendBlePrompterCommand(`C${selectedSuitCode}${button.dataset.rankCode}`);
  });
}

symbolInput.addEventListener("input", () => {
  symbolInput.value = symbolInput.value.slice(0, 2).toUpperCase();
});

symbolForm.addEventListener("submit", (event) => {
  event.preventDefault();
  const symbolText = symbolInput.value.trim().slice(0, 2).toUpperCase();

  if (!symbolText) {
    lastResponseOutput.value = "Symbol fehlt.";
    addLogEntry("Symbol fehlt.");
    return;
  }

  sendBlePrompterCommand(`SYMBOL ${symbolText}`);
});

resetBluetoothState();
