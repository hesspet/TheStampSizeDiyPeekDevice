import {
  MAXIMUM_CAMERA_BUTTON_OPACITY,
  MAXIMUM_CAMERA_ZOOM,
  MINIMUM_CAMERA_BUTTON_OPACITY,
} from './constants';
import type { CameraButtonAppearance, DiscoveredBleDevice } from './types';

export function formatRecordingDuration(totalSeconds: number) {
  const minutes = Math.floor(totalSeconds / 60);
  const seconds = totalSeconds % 60;

  return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
}

export function getDeviceDisplayName(device: DiscoveredBleDevice) {
  return `${device.name} (${device.id})`;
}

export function clampCameraZoom(value: number) {
  return Math.max(0, Math.min(MAXIMUM_CAMERA_ZOOM, value));
}

export function clampCameraButtonOpacity(value: number) {
  return Math.max(MINIMUM_CAMERA_BUTTON_OPACITY, Math.min(MAXIMUM_CAMERA_BUTTON_OPACITY, value));
}

export function getCameraButtonOpacity(
  selectedCameraButtonAppearance: CameraButtonAppearance,
  customCameraButtonOpacity: number
) {
  if (selectedCameraButtonAppearance === 'normal') {
    return 1;
  }

  if (selectedCameraButtonAppearance === 'transparent') {
    return 0.72;
  }

  if (selectedCameraButtonAppearance === 'minimal') {
    return 0.28;
  }

  return clampCameraButtonOpacity(customCameraButtonOpacity);
}
