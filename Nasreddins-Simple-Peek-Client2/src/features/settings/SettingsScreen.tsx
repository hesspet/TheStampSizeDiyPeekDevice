import type { Dispatch, SetStateAction } from 'react';
import type { GestureResponderEvent, LayoutChangeEvent } from 'react-native';
import { Pressable, Switch, Text, View } from 'react-native';

import { cameraButtonAppearanceOptions, cameraViewStyleOptions } from '@/shared/constants';
import { clampCameraButtonOpacity } from '@/shared/formatters';
import type { AppStyles } from '@/shared/styles';
import type { CameraButtonAppearance, CameraViewStyle } from '@/shared/types';

type SettingsScreenProps = {
  cameraButtonOpacityText: string;
  cameraButtonOpacityTrackFillWidth: number;
  cameraButtonOpacitySliderWidth: number;
  customCameraButtonOpacity: number;
  isDarkModeEnabled: boolean;
  isDisplayInverted: boolean;
  isDisplayRotated: boolean;
  selectedCameraButtonAppearance: CameraButtonAppearance;
  selectedCameraViewStyle: CameraViewStyle;
  setCameraButtonOpacitySliderWidth: (width: number) => void;
  setCustomCameraButtonOpacity: Dispatch<SetStateAction<number>>;
  setDisplayInverted: (nextValue: boolean) => void;
  setDisplayRotated: (nextValue: boolean) => void;
  setIsDarkModeEnabled: Dispatch<SetStateAction<boolean>>;
  setSelectedCameraButtonAppearance: Dispatch<SetStateAction<CameraButtonAppearance>>;
  setSelectedCameraViewStyle: Dispatch<SetStateAction<CameraViewStyle>>;
  setShouldCloseButtonViewAfterSend: Dispatch<SetStateAction<boolean>>;
  shouldCloseButtonViewAfterSend: boolean;
  shouldShowCameraButtonOpacitySlider: boolean;
  styles: AppStyles;
};

/**
 * Eigenständiger Menüpunkt für Einstellungen.
 * Neue Einstellungen sollten hier landen, solange sie nicht zu einem eigenen Feature gehören.
 */
export function SettingsScreen({
  cameraButtonOpacityText,
  cameraButtonOpacityTrackFillWidth,
  cameraButtonOpacitySliderWidth,
  isDarkModeEnabled,
  isDisplayInverted,
  isDisplayRotated,
  selectedCameraButtonAppearance,
  selectedCameraViewStyle,
  setCameraButtonOpacitySliderWidth,
  setCustomCameraButtonOpacity,
  setDisplayInverted,
  setDisplayRotated,
  setIsDarkModeEnabled,
  setSelectedCameraButtonAppearance,
  setSelectedCameraViewStyle,
  setShouldCloseButtonViewAfterSend,
  shouldCloseButtonViewAfterSend,
  shouldShowCameraButtonOpacitySlider,
  styles,
}: SettingsScreenProps) {
  const updateCameraButtonOpacityFromSlider = (event: GestureResponderEvent) => {
    if (cameraButtonOpacitySliderWidth <= 0) {
      return;
    }

    const opacityRatio = event.nativeEvent.locationX / cameraButtonOpacitySliderWidth;
    setCustomCameraButtonOpacity(clampCameraButtonOpacity(opacityRatio));
  };

  const updateCameraButtonOpacitySliderWidth = (event: LayoutChangeEvent) => {
    setCameraButtonOpacitySliderWidth(event.nativeEvent.layout.width);
  };

  return (
    <View style={styles.panel}>
      <View style={styles.toggleControl}>
        <Text style={styles.toggleText}>Dunkelmodus</Text>
        <Switch onValueChange={setIsDarkModeEnabled} value={isDarkModeEnabled} />
      </View>
      <View style={styles.toggleControl}>
        <Text style={styles.toggleText}>Schließe Buttonansicht nach Senden</Text>
        <Switch
          onValueChange={setShouldCloseButtonViewAfterSend}
          value={shouldCloseButtonViewAfterSend}
        />
      </View>
      <View style={styles.settingsGroup}>
        <Text style={styles.inputLabel}>Display</Text>
        <View style={styles.toggleControl}>
          <Text style={styles.toggleText}>Displayanzeige invertiert</Text>
          <Switch onValueChange={setDisplayInverted} value={isDisplayInverted} />
        </View>
        <View style={styles.toggleControl}>
          <Text style={styles.toggleText}>Displayanzeige drehen</Text>
          <Switch onValueChange={setDisplayRotated} value={isDisplayRotated} />
        </View>
      </View>
      <View style={styles.settingsGroup}>
        <Text style={styles.inputLabel}>Kameraansicht</Text>
        <View style={styles.cameraStyleOptions}>
          {cameraViewStyleOptions.map((option) => (
            <Pressable
              accessibilityRole="button"
              accessibilityState={{ selected: selectedCameraViewStyle === option.value }}
              key={option.value}
              onPress={() => setSelectedCameraViewStyle(option.value)}
              style={[
                styles.cameraStyleOption,
                selectedCameraViewStyle === option.value && styles.activeModeTab,
              ]}>
              <Text
                style={[
                  styles.modeTabText,
                  selectedCameraViewStyle === option.value && styles.activeModeTabText,
                ]}>
                {option.label}
              </Text>
            </Pressable>
          ))}
        </View>
      </View>
      <View style={styles.settingsGroup}>
        <Text style={styles.inputLabel}>Kamera-Bedienung</Text>
        <View style={styles.cameraStyleOptions}>
          {cameraButtonAppearanceOptions.map((option) => (
            <Pressable
              accessibilityRole="button"
              accessibilityState={{ selected: selectedCameraButtonAppearance === option.value }}
              key={option.value}
              onPress={() => setSelectedCameraButtonAppearance(option.value)}
              style={[
                styles.cameraStyleOption,
                selectedCameraButtonAppearance === option.value && styles.activeModeTab,
              ]}>
              <Text
                style={[
                  styles.modeTabText,
                  selectedCameraButtonAppearance === option.value && styles.activeModeTabText,
                ]}>
                {option.label}
              </Text>
            </Pressable>
          ))}
        </View>
        {shouldShowCameraButtonOpacitySlider ? (
          <View style={styles.opacitySliderGroup}>
            <View style={styles.opacitySliderHeader}>
              <Text style={styles.toggleText}>Deckkraft</Text>
              <Text style={styles.opacitySliderValue}>{cameraButtonOpacityText}</Text>
            </View>
            <View
              accessibilityLabel="Deckkraft der Kamera-Bedienung"
              accessibilityRole="adjustable"
              onLayout={updateCameraButtonOpacitySliderWidth}
              onMoveShouldSetResponder={() => true}
              onResponderGrant={updateCameraButtonOpacityFromSlider}
              onResponderMove={updateCameraButtonOpacityFromSlider}
              onStartShouldSetResponder={() => true}
              style={styles.opacitySliderTrack}>
              <View
                pointerEvents="none"
                style={[styles.opacitySliderFill, { width: cameraButtonOpacityTrackFillWidth }]}
              />
              <View
                pointerEvents="none"
                style={[styles.opacitySliderThumb, { left: cameraButtonOpacityTrackFillWidth }]}
              />
            </View>
          </View>
        ) : null}
      </View>
    </View>
  );
}
