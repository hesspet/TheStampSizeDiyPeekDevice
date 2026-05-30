import type { Device } from 'react-native-ble-plx';

export type ConnectionStatus =
  | 'Nicht verbunden'
  | 'Berechtigung fehlt'
  | 'Bluetooth aus'
  | 'Suche Gerät'
  | 'Verbinde'
  | 'Verbunden'
  | 'Testmodus'
  | 'Getrennt'
  | 'Fehler';

export type BleScanMode = 'filtered' | 'allDevices';
export type CameraViewStyle = 'android' | 'ios' | 'fantasy';
export type CameraButtonAppearance = 'normal' | 'transparent' | 'minimal' | 'custom';
export type ModeLabel = 'Pfeile' | 'Karten' | 'Symbole' | 'Raw' | 'ESP';
export type MenuItem = 'Client' | 'Einstellungen' | 'Log' | 'About';

export type DiscoveredBleDevice = {
  id: string;
  name: string;
  device: Device;
};

export type RememberedBleDevice = {
  id: string;
  name: string;
};

export type DirectionCommand = {
  label: string;
  arrow: string;
  command: string;
};

export type SuitOption = {
  label: string;
  mark: string;
  code: string;
  isRed: boolean;
};

export type RankOption = {
  label: string;
  code: string;
};

export type EspSymbolCommand = {
  label: string;
  command: string;
  symbol: string;
};
