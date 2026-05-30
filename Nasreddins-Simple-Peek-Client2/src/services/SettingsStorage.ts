import AsyncStorage from '@react-native-async-storage/async-storage';

import { clampCameraButtonOpacity } from '@/shared/formatters';
import type { CameraButtonAppearance, CameraViewStyle, RememberedBleDevice } from '@/shared/types';

const SETTINGS_STORAGE_KEY = 'nasreddins-simple-peek-client2:settings:v1';

const cameraViewStyles: CameraViewStyle[] = ['android', 'ios', 'fantasy'];
const cameraButtonAppearances: CameraButtonAppearance[] = [
  'normal',
  'transparent',
  'minimal',
  'custom',
];

export type PersistedSettings = {
  customCameraButtonOpacity: number;
  isDarkModeEnabled: boolean;
  isDisplayInverted: boolean;
  isDisplayRotated: boolean;
  rememberedBleDevice: RememberedBleDevice | null;
  selectedCameraButtonAppearance: CameraButtonAppearance;
  selectedCameraViewStyle: CameraViewStyle;
  shouldCloseButtonViewAfterSend: boolean;
};

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === 'object' && value !== null;
}

function isRememberedBleDevice(value: unknown): value is RememberedBleDevice {
  return (
    isRecord(value) &&
    typeof value.id === 'string' &&
    value.id.length > 0 &&
    typeof value.name === 'string' &&
    value.name.length > 0
  );
}

function isCameraViewStyle(value: unknown): value is CameraViewStyle {
  return typeof value === 'string' && cameraViewStyles.includes(value as CameraViewStyle);
}

function isCameraButtonAppearance(value: unknown): value is CameraButtonAppearance {
  return (
    typeof value === 'string' &&
    cameraButtonAppearances.includes(value as CameraButtonAppearance)
  );
}

export async function loadPersistedSettings() {
  const settingsText = await AsyncStorage.getItem(SETTINGS_STORAGE_KEY);

  if (!settingsText) {
    return null;
  }

  const parsedSettings: unknown = JSON.parse(settingsText);

  if (!isRecord(parsedSettings)) {
    return null;
  }

  const persistedSettings: Partial<PersistedSettings> = {};

  if (typeof parsedSettings.isDarkModeEnabled === 'boolean') {
    persistedSettings.isDarkModeEnabled = parsedSettings.isDarkModeEnabled;
  }

  if (typeof parsedSettings.isDisplayInverted === 'boolean') {
    persistedSettings.isDisplayInverted = parsedSettings.isDisplayInverted;
  }

  if (typeof parsedSettings.isDisplayRotated === 'boolean') {
    persistedSettings.isDisplayRotated = parsedSettings.isDisplayRotated;
  }

  if (typeof parsedSettings.shouldCloseButtonViewAfterSend === 'boolean') {
    persistedSettings.shouldCloseButtonViewAfterSend =
      parsedSettings.shouldCloseButtonViewAfterSend;
  }

  if (isCameraViewStyle(parsedSettings.selectedCameraViewStyle)) {
    persistedSettings.selectedCameraViewStyle = parsedSettings.selectedCameraViewStyle;
  }

  if (isCameraButtonAppearance(parsedSettings.selectedCameraButtonAppearance)) {
    persistedSettings.selectedCameraButtonAppearance =
      parsedSettings.selectedCameraButtonAppearance;
  }

  if (
    typeof parsedSettings.customCameraButtonOpacity === 'number' &&
    Number.isFinite(parsedSettings.customCameraButtonOpacity)
  ) {
    persistedSettings.customCameraButtonOpacity = clampCameraButtonOpacity(
      parsedSettings.customCameraButtonOpacity
    );
  }

  if (isRememberedBleDevice(parsedSettings.rememberedBleDevice)) {
    persistedSettings.rememberedBleDevice = parsedSettings.rememberedBleDevice;
  } else if (parsedSettings.rememberedBleDevice === null) {
    persistedSettings.rememberedBleDevice = null;
  }

  return persistedSettings;
}

export async function savePersistedSettings(settings: PersistedSettings) {
  await AsyncStorage.setItem(SETTINGS_STORAGE_KEY, JSON.stringify(settings));
}
