import type {
  CameraButtonAppearance,
  CameraViewStyle,
  DirectionCommand,
  EspSymbolCommand,
  MenuItem,
  ModeLabel,
  RankOption,
  SuitOption,
} from './types';

export const BLE_PROMPTER_SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
export const BLE_PROMPTER_RECEIVE_CHARACTERISTIC_UUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
export const BLE_PROMPTER_TRANSMIT_CHARACTERISTIC_UUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
export const BLE_PROMPTER_NAME_PREFIX = 'BlePrompter';
export const MAXIMUM_LOG_ENTRIES = 8;
export const MAXIMUM_CAMERA_ZOOM = 0.7;
export const MINIMUM_CAMERA_BUTTON_OPACITY = 0.05;
export const MAXIMUM_CAMERA_BUTTON_OPACITY = 1;

export const directionCommands: DirectionCommand[] = [
  { label: 'NW', arrow: '↖', command: 'ANW' },
  { label: 'N', arrow: '↑', command: 'AN' },
  { label: 'NO', arrow: '↗', command: 'ANE' },
  { label: 'W', arrow: '←', command: 'AW' },
  { label: 'O', arrow: '→', command: 'AE' },
  { label: 'SW', arrow: '↙', command: 'ASW' },
  { label: 'S', arrow: '↓', command: 'AS' },
  { label: 'SO', arrow: '↘', command: 'ASE' },
];

export const suitOptions: SuitOption[] = [
  { label: 'Herz', mark: '♥', code: 'H', isRed: true },
  { label: 'Karo', mark: '♦', code: 'D', isRed: true },
  { label: 'Kreuz', mark: '♣', code: 'C', isRed: false },
  { label: 'Pik', mark: '♠', code: 'S', isRed: false },
];

export const rankOptions: RankOption[] = [
  { label: 'Ass', code: '1' },
  { label: '2', code: '2' },
  { label: '3', code: '3' },
  { label: '4', code: '4' },
  { label: '5', code: '5' },
  { label: '6', code: '6' },
  { label: '7', code: '7' },
  { label: '8', code: '8' },
  { label: '9', code: '9' },
  { label: '10', code: 'X' },
  { label: 'Bube', code: 'J' },
  { label: 'Dame', code: 'Q' },
  { label: 'König', code: 'K' },
];

export const espSymbolCommands: EspSymbolCommand[] = [
  { label: 'Kreis', command: 'EC', symbol: '○' },
  { label: 'Kreuz', command: 'EG', symbol: '+' },
  { label: 'Wellen', command: 'EW', symbol: '≋' },
  { label: 'Quadrat', command: 'EQ', symbol: '□' },
  { label: 'Stern', command: 'ES', symbol: '☆' },
];

export const modeLabels: ModeLabel[] = ['Pfeile', 'Karten', 'Symbole', 'Raw', 'ESP'];
export const menuItems: MenuItem[] = ['Client', 'Einstellungen', 'Log', 'About'];
export const menuItemLabels: Record<MenuItem, string> = {
  Client: 'Peeker (Videofake)',
  Einstellungen: 'Einstellungen',
  Log: 'Log',
  About: 'About',
};

export const cameraViewStyleOptions: { label: string; value: CameraViewStyle }[] = [
  { label: 'Android', value: 'android' },
  { label: 'iOS', value: 'ios' },
  { label: 'Fantasie', value: 'fantasy' },
];

export const cameraButtonAppearanceOptions: {
  label: string;
  value: CameraButtonAppearance;
}[] = [
  { label: 'Normal', value: 'normal' },
  { label: 'Durchsichtig', value: 'transparent' },
  { label: 'Unauffällig', value: 'minimal' },
  { label: 'Anpassbar', value: 'custom' },
];
