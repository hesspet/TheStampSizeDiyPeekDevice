import { PermissionsAndroid, Platform } from 'react-native';
import { BleError, BleManager, Characteristic, State } from 'react-native-ble-plx';

import { decodeBase64AsText, encodeTextAsBase64 } from '@/shared/base64Text';
import {
  BLE_PROMPTER_NAME_PREFIX,
  BLE_PROMPTER_RECEIVE_CHARACTERISTIC_UUID,
  BLE_PROMPTER_SERVICE_UUID,
  BLE_PROMPTER_TRANSMIT_CHARACTERISTIC_UUID,
} from '@/shared/constants';
import type {
  BleScanMode,
  ConnectionStatus,
  DiscoveredBleDevice,
  RememberedBleDevice,
} from '@/shared/types';

type BlePrompterServiceEvents = {
  onDevicesChanged: (devices: DiscoveredBleDevice[]) => void;
  onLog: (message: string) => void;
  onResponse: (message: string) => void;
  onStatusChanged: (status: ConnectionStatus) => void;
  onConnectionChanged: (isConnected: boolean, connectedDeviceName: string) => void;
  onScanningChanged: (isScanning: boolean) => void;
};

export function getBleErrorMessage(error: unknown) {
  if (error instanceof BleError) {
    if (error.errorCode === 102) {
      return 'Bluetooth ist ausgeschaltet.';
    }

    if (error.errorCode === 201) {
      return 'Die BLE-Verbindung ist fehlgeschlagen.';
    }

    if (error.errorCode === 203) {
      return 'Gerät wurde getrennt.';
    }
  }

  return 'Aktion fehlgeschlagen.';
}

async function requestAndroidBluetoothPermissions() {
  if (Platform.OS !== 'android' || Platform.Version < 31) {
    return true;
  }

  const permissionResults = await PermissionsAndroid.requestMultiple([
    PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
    PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
  ]);

  return (
    permissionResults[PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN] ===
      PermissionsAndroid.RESULTS.GRANTED &&
    permissionResults[PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT] ===
      PermissionsAndroid.RESULTS.GRANTED
  );
}

/**
 * Kapselt alle direkten BLE-Zugriffe.
 * React-Komponenten arbeiten nur mit deutschen Statusmeldungen und Befehlen,
 * während diese Klasse Scan, Verbindung und UART-Schreibzugriff verwaltet.
 */
export class BlePrompterService {
  private readonly bleManager = new BleManager();
  private discoveredDevices: DiscoveredBleDevice[] = [];
  private disconnectedSubscription: { remove: () => void } | null = null;
  private events: BlePrompterServiceEvents;
  private notificationSubscription: { remove: () => void } | null = null;
  private receiveCharacteristic: Characteristic | null = null;
  private scanTimeoutId: ReturnType<typeof setTimeout> | null = null;

  constructor(events: BlePrompterServiceEvents) {
    this.events = events;
  }

  updateEvents(events: BlePrompterServiceEvents) {
    this.events = events;
  }

  async scanForDevices(mode: BleScanMode) {
    const hasPermissions = await requestAndroidBluetoothPermissions();

    if (!hasPermissions) {
      this.events.onStatusChanged('Berechtigung fehlt');
      this.events.onResponse('Bluetooth-Berechtigung wurde nicht erteilt.');
      this.events.onLog('Bluetooth-Berechtigung wurde nicht erteilt.');
      return;
    }

    const bluetoothState = await this.bleManager.state();

    if (bluetoothState !== State.PoweredOn) {
      this.events.onStatusChanged('Bluetooth aus');
      this.events.onResponse('Bitte Bluetooth am Android-Gerät einschalten.');
      this.events.onLog('Bluetooth ist ausgeschaltet.');
      return;
    }

    this.discoveredDevices = [];
    this.events.onDevicesChanged([]);
    this.events.onStatusChanged('Suche Gerät');
    this.events.onScanningChanged(true);
    this.bleManager.stopDeviceScan();

    const serviceFilter = mode === 'filtered' ? [BLE_PROMPTER_SERVICE_UUID] : null;

    this.bleManager.startDeviceScan(serviceFilter, null, (error, device) => {
      if (error) {
        const errorMessage = getBleErrorMessage(error);
        this.events.onStatusChanged('Fehler');
        this.events.onResponse(errorMessage);
        this.events.onScanningChanged(false);
        this.events.onLog(errorMessage);
        this.bleManager.stopDeviceScan();
        return;
      }

      if (!device) {
        return;
      }

      const deviceName = device.name || device.localName || 'Unbekanntes Gerät';
      const matchesFilteredScan =
        deviceName.startsWith(BLE_PROMPTER_NAME_PREFIX) ||
        Boolean(device.serviceUUIDs?.includes(BLE_PROMPTER_SERVICE_UUID));

      if (mode === 'filtered' && !matchesFilteredScan) {
        return;
      }

      if (this.discoveredDevices.some((currentDevice) => currentDevice.id === device.id)) {
        return;
      }

      this.discoveredDevices = [...this.discoveredDevices, { id: device.id, name: deviceName, device }];
      this.events.onDevicesChanged(this.discoveredDevices);
    });

    this.scanTimeoutId = setTimeout(() => {
      this.stopScan();
    }, 10000);
  }

  async connectToDevice(discoveredDevice: DiscoveredBleDevice) {
    this.events.onStatusChanged('Verbinde');
    this.events.onScanningChanged(false);
    this.bleManager.stopDeviceScan();

    try {
      const connectedDevice = await discoveredDevice.device.connect();
      const deviceWithServices = await connectedDevice.discoverAllServicesAndCharacteristics();
      const serviceCharacteristics = await deviceWithServices.characteristicsForService(
        BLE_PROMPTER_SERVICE_UUID
      );
      const receiveWriteCharacteristic = serviceCharacteristics.find(
        (characteristic) =>
          characteristic.uuid.toLowerCase() === BLE_PROMPTER_RECEIVE_CHARACTERISTIC_UUID
      );

      if (!receiveWriteCharacteristic) {
        throw new Error('Receive characteristic missing');
      }

      this.notificationSubscription = deviceWithServices.monitorCharacteristicForService(
        BLE_PROMPTER_SERVICE_UUID,
        BLE_PROMPTER_TRANSMIT_CHARACTERISTIC_UUID,
        (error, characteristic) => {
          if (error) {
            const errorMessage = getBleErrorMessage(error);
            this.events.onResponse(errorMessage);
            this.events.onLog(errorMessage);
            return;
          }

          if (!characteristic?.value) {
            return;
          }

          const responseText = decodeBase64AsText(characteristic.value);
          this.events.onResponse(responseText);
          this.events.onLog(`Antwort: ${responseText}`);
        }
      );

      this.disconnectedSubscription = connectedDevice.onDisconnected(() => {
        this.resetBluetoothState('Getrennt');
        this.events.onLog('Verbindung getrennt.');
      });

      this.receiveCharacteristic = receiveWriteCharacteristic;
      this.discoveredDevices = [];
      this.events.onDevicesChanged([]);
      this.events.onConnectionChanged(true, discoveredDevice.name);
      this.events.onStatusChanged('Verbunden');
      this.events.onResponse('-');
      this.events.onLog(`Verbunden mit ${discoveredDevice.name}.`);
      return true;
    } catch (error) {
      const errorMessage = getBleErrorMessage(error);
      this.resetBluetoothState('Fehler');
      this.events.onResponse(errorMessage);
      this.events.onLog(errorMessage);
      return false;
    }
  }

  async reconnectToRememberedDevice(
    rememberedDevice: RememberedBleDevice,
    shouldContinueReconnect: () => boolean
  ) {
    const hasPermissions = await requestAndroidBluetoothPermissions();

    if (!shouldContinueReconnect()) {
      this.resetBluetoothState('Nicht verbunden');
      return false;
    }

    if (!hasPermissions) {
      this.events.onStatusChanged('Berechtigung fehlt');
      this.events.onResponse('Bluetooth-Berechtigung wurde nicht erteilt.');
      this.events.onLog('Automatische Verbindung abgebrochen: Berechtigung fehlt.');
      return false;
    }

    const bluetoothState = await this.bleManager.state();

    if (!shouldContinueReconnect()) {
      this.resetBluetoothState('Nicht verbunden');
      return false;
    }

    if (bluetoothState !== State.PoweredOn) {
      this.events.onStatusChanged('Bluetooth aus');
      this.events.onResponse('Bitte Bluetooth am Android-Gerät einschalten.');
      this.events.onLog('Automatische Verbindung abgebrochen: Bluetooth ist ausgeschaltet.');
      return false;
    }

    this.discoveredDevices = [];
    this.events.onDevicesChanged([]);
    this.events.onStatusChanged('Suche Gerät');
    this.events.onResponse(`Suche ${rememberedDevice.name}.`);
    this.events.onScanningChanged(true);
    this.events.onLog(`Suche zuletzt genutztes Gerät: ${rememberedDevice.name}.`);
    this.bleManager.stopDeviceScan();

    return new Promise<boolean>((resolve) => {
      let isFinished = false;

      const finishReconnect = (wasConnected: boolean) => {
        if (isFinished) {
          return;
        }

        isFinished = true;
        this.stopScan();

        if (!wasConnected) {
          this.resetBluetoothState('Nicht verbunden');
        }

        resolve(wasConnected);
      };

      this.scanTimeoutId = setTimeout(() => {
        this.events.onResponse(`${rememberedDevice.name} wurde nicht gefunden.`);
        this.events.onLog(`Zuletzt genutztes Gerät nicht gefunden: ${rememberedDevice.name}.`);
        finishReconnect(false);
      }, 10000);

      this.bleManager.startDeviceScan([BLE_PROMPTER_SERVICE_UUID], null, async (error, device) => {
        if (!shouldContinueReconnect()) {
          this.events.onLog('Automatische Verbindung wurde abgebrochen.');
          finishReconnect(false);
          return;
        }

        if (error) {
          const errorMessage = getBleErrorMessage(error);
          this.events.onStatusChanged('Fehler');
          this.events.onResponse(errorMessage);
          this.events.onLog(errorMessage);
          finishReconnect(false);
          return;
        }

        if (!device) {
          return;
        }

        const deviceName = device.name || device.localName || 'Unbekanntes Gerät';
        const isRememberedDevice =
          device.id === rememberedDevice.id || deviceName === rememberedDevice.name;

        if (!isRememberedDevice) {
          return;
        }

        if (isFinished) {
          return;
        }

        isFinished = true;
        this.stopScan();
        resolve(await this.connectToDevice({ id: device.id, name: deviceName, device }));
      });
    });
  }

  async disconnect() {
    if (!this.receiveCharacteristic?.deviceID) {
      this.resetBluetoothState('Nicht verbunden');
      return;
    }

    try {
      await this.bleManager.cancelDeviceConnection(this.receiveCharacteristic.deviceID);
    } catch {
      this.resetBluetoothState('Getrennt');
    }
  }

  async sendCommand(command: string) {
    if (!this.receiveCharacteristic) {
      const message = 'BlePrompter ist nicht verbunden.';
      this.events.onResponse(message);
      this.events.onLog(message);
      return false;
    }

    try {
      await this.receiveCharacteristic.writeWithoutResponse(encodeTextAsBase64(command));
      this.events.onLog(`Gesendet: ${command}`);
      return true;
    } catch {
      try {
        await this.receiveCharacteristic.writeWithResponse(encodeTextAsBase64(command));
        this.events.onLog(`Gesendet: ${command}`);
        return true;
      } catch (error) {
        const errorMessage = getBleErrorMessage(error);
        this.events.onResponse(errorMessage);
        this.events.onLog(errorMessage);
        return false;
      }
    }
  }

  resetBluetoothState(status: ConnectionStatus = 'Nicht verbunden') {
    this.notificationSubscription?.remove();
    this.disconnectedSubscription?.remove();
    this.notificationSubscription = null;
    this.disconnectedSubscription = null;
    this.receiveCharacteristic = null;
    this.discoveredDevices = [];
    this.stopScan();
    this.events.onScanningChanged(false);
    this.events.onConnectionChanged(false, '');
    this.events.onDevicesChanged([]);
    this.events.onStatusChanged(status);
  }

  async destroy() {
    this.notificationSubscription?.remove();
    this.disconnectedSubscription?.remove();
    this.notificationSubscription = null;
    this.disconnectedSubscription = null;
    this.receiveCharacteristic = null;
    this.stopScan();
    this.bleManager.destroy();
  }

  stopScan() {
    if (this.scanTimeoutId) {
      clearTimeout(this.scanTimeoutId);
      this.scanTimeoutId = null;
    }

    this.bleManager.stopDeviceScan();
    this.events.onScanningChanged(false);
  }
}
