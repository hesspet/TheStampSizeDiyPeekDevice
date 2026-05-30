import { Pressable, Text } from 'react-native';

import type { AppStyles } from '@/shared/styles';

type CommandButtonProps = {
  cameraButtonOpacity?: number;
  disabled?: boolean;
  icon?: string;
  isCameraOverlay?: boolean;
  isPrimary?: boolean;
  isSelected?: boolean;
  label: string;
  onPress: () => void | Promise<unknown>;
  shouldUseHiddenCameraButtonAppearance?: boolean;
  styles: AppStyles;
};

/**
 * Einheitliche Befehlstaste für normale Ansichten und Kamera-Overlay.
 * Dadurch sehen alle Programmteile gleich aus und neue Module müssen
 * keine eigene Button-Logik kopieren.
 */
export function CommandButton({
  cameraButtonOpacity = 1,
  disabled,
  icon,
  isCameraOverlay,
  isPrimary,
  isSelected,
  label,
  onPress,
  shouldUseHiddenCameraButtonAppearance,
  styles,
}: CommandButtonProps) {
  const isDisabled = Boolean(disabled);
  const shouldUseTransparentButton =
    Boolean(isCameraOverlay) && Boolean(shouldUseHiddenCameraButtonAppearance);

  return (
    <Pressable
      accessibilityRole="button"
      disabled={isDisabled}
      onPress={onPress}
      style={({ pressed }) => [
        styles.button,
        isPrimary && styles.primaryButton,
        isSelected && styles.selectedButton,
        shouldUseTransparentButton && styles.transparentCameraButton,
        shouldUseTransparentButton && { opacity: cameraButtonOpacity },
        isDisabled && styles.disabledButton,
        pressed && !isDisabled && styles.pressedButton,
      ]}>
      {icon ? (
        <Text style={[styles.buttonIcon, shouldUseTransparentButton && styles.transparentCameraButtonIcon]}>
          {icon}
        </Text>
      ) : null}
      <Text
        style={[
          styles.buttonText,
          isPrimary && styles.primaryButtonText,
          shouldUseTransparentButton && styles.transparentCameraButtonText,
        ]}>
        {label}
      </Text>
    </Pressable>
  );
}
