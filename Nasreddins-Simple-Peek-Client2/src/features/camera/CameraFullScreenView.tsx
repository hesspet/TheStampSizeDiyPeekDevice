import { CameraView, type PermissionResponse } from 'expo-camera';
import { useRef } from 'react';
import { Pressable, ScrollView, Text, View } from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import { CommandButton } from '@/features/commands/CommandButton';
import { CommandControls } from '@/features/commands/CommandControls';
import { cameraViewStyleOptions } from '@/shared/constants';
import { clampCameraZoom } from '@/shared/formatters';
import type { AppStyles } from '@/shared/styles';
import type { CameraViewStyle, ModeLabel } from '@/shared/types';

import { getPinchDistance } from './cameraGestures';

type CameraFullScreenViewProps = {
  areFullScreenControlsVisible: boolean;
  cameraButtonOpacity: number;
  cameraPermission: PermissionResponse | null;
  cameraZoom: number;
  cameraZoomText: string;
  closeFullScreenMode: () => void;
  compassGridSize: number;
  controlsAreDisabled: boolean;
  fullScreenMode: ModeLabel;
  isFakeVideoPaused: boolean;
  isLandscapeOrientation: boolean;
  rawCommandText: string;
  recordingDurationText: string;
  requestCameraPermission: () => Promise<PermissionResponse>;
  selectedCameraViewStyle: CameraViewStyle;
  selectedSuitCode: string;
  setAreFullScreenControlsVisible: (nextValue: boolean) => void;
  setCameraZoom: (nextValue: number) => void;
  setIsFakeVideoPaused: (nextValue: boolean | ((currentValue: boolean) => boolean)) => void;
  setRawCommandText: (rawCommandText: string) => void;
  setSymbolText: (symbolText: string) => void;
  shouldUseHiddenCameraButtonAppearance: boolean;
  styles: AppStyles;
  symbolText: string;
  onClearDisplay: () => void | Promise<unknown>;
  onSelectCardSuit: (suitCode: string) => void;
  onSendCardRank: (rankCode: string) => void | Promise<unknown>;
  onSendCommand: (command: string) => void | Promise<unknown>;
  onSendRawCommand: () => void | Promise<unknown>;
  onSendSymbol: () => void | Promise<unknown>;
};

/**
 * Eigenständiges Kamera-Modul mit Livebild, Zoom-Geste und Overlay-Bedienung.
 * Es enthält keine BLE-Logik, sondern ruft nur die übergebenen Befehlsfunktionen auf.
 */
export function CameraFullScreenView({
  areFullScreenControlsVisible,
  cameraButtonOpacity,
  cameraPermission,
  cameraZoom,
  cameraZoomText,
  closeFullScreenMode,
  compassGridSize,
  controlsAreDisabled,
  fullScreenMode,
  isFakeVideoPaused,
  isLandscapeOrientation,
  onClearDisplay,
  onSelectCardSuit,
  onSendCardRank,
  onSendCommand,
  onSendRawCommand,
  onSendSymbol,
  rawCommandText,
  recordingDurationText,
  requestCameraPermission,
  selectedCameraViewStyle,
  selectedSuitCode,
  setAreFullScreenControlsVisible,
  setCameraZoom,
  setIsFakeVideoPaused,
  setRawCommandText,
  setSymbolText,
  shouldUseHiddenCameraButtonAppearance,
  styles,
  symbolText,
}: CameraFullScreenViewProps) {
  const startingPinchDistance = useRef<number | null>(null);
  const startingPinchZoom = useRef(0);
  const hasCameraPermission = Boolean(cameraPermission?.granted);
  const selectedCameraViewStyleLabel =
    cameraViewStyleOptions.find((option) => option.value === selectedCameraViewStyle)?.label ??
    'Android';

  const startCameraZoomGesture = (event: Parameters<typeof getPinchDistance>[0]) => {
    const pinchDistance = getPinchDistance(event);

    if (!pinchDistance) {
      return false;
    }

    startingPinchDistance.current = pinchDistance;
    startingPinchZoom.current = cameraZoom;
    return true;
  };

  const updateCameraZoomGesture = (event: Parameters<typeof getPinchDistance>[0]) => {
    const pinchDistance = getPinchDistance(event);

    if (!pinchDistance || !startingPinchDistance.current) {
      return;
    }

    const pinchDelta = (pinchDistance - startingPinchDistance.current) / 420;
    setCameraZoom(clampCameraZoom(startingPinchZoom.current + pinchDelta));
  };

  const endCameraZoomGesture = () => {
    startingPinchDistance.current = null;
  };

  return (
    <SafeAreaView style={styles.cameraSafeArea}>
      <View style={styles.cameraScreen}>
        {hasCameraPermission ? (
          <CameraView active facing="back" style={styles.cameraPreview} zoom={cameraZoom} />
        ) : (
          <View style={styles.cameraFallback}>
            <Text style={styles.cameraFallbackTitle}>Kamerazugriff fehlt.</Text>
            <Text style={styles.cameraFallbackText}>
              Erlaube den Kamerazugriff, damit die Liveansicht angezeigt werden kann.
            </Text>
            <CommandButton isPrimary label="Kamera erlauben" onPress={requestCameraPermission} styles={styles} />
            <CommandButton
              label="Bedienung einblenden"
              onPress={() => setAreFullScreenControlsVisible(true)}
              styles={styles}
            />
            <CommandButton label="Zurück" onPress={closeFullScreenMode} styles={styles} />
          </View>
        )}

        <View pointerEvents="none" style={styles.viewfinderLayer}>
          <View style={styles.viewfinderHorizontalLine} />
          <View style={styles.viewfinderVerticalLine} />
          <View style={styles.viewfinderFrame} />
        </View>

        {hasCameraPermission ? (
          <View
            onMoveShouldSetResponder={(event) => event.nativeEvent.touches.length >= 2}
            onResponderGrant={startCameraZoomGesture}
            onResponderMove={updateCameraZoomGesture}
            onResponderRelease={endCameraZoomGesture}
            onResponderTerminate={endCameraZoomGesture}
            onStartShouldSetResponder={(event) => event.nativeEvent.touches.length >= 2}
            style={styles.cameraGestureLayer}
          />
        ) : null}

        {isFakeVideoPaused ? (
          <View pointerEvents="none" style={styles.fakePauseOverlay}>
            <Text style={styles.fakePauseText}>PAUSE</Text>
          </View>
        ) : null}

        <View
          style={[
            styles.cameraHud,
            selectedCameraViewStyle === 'ios' && styles.cameraHudIos,
            selectedCameraViewStyle === 'fantasy' && styles.cameraHudFantasy,
          ]}>
          <View style={styles.recordingBadge}>
            <View style={styles.recordingDot} />
            <Text style={styles.recordingText}>REC {recordingDurationText}</Text>
          </View>
          <Pressable
            accessibilityLabel="Bedienung einblenden"
            accessibilityRole="button"
            disabled={!hasCameraPermission}
            onPress={() => setAreFullScreenControlsVisible(true)}
            style={({ pressed }) => [
              styles.cameraTitleButton,
              pressed && hasCameraPermission && styles.pressedButton,
            ]}>
            <Text style={styles.cameraHudTitleText}>{fullScreenMode}</Text>
          </Pressable>
          <Text style={styles.cameraHudText}>{selectedCameraViewStyleLabel}</Text>
        </View>

        {areFullScreenControlsVisible ? (
          <View
            style={[
              styles.fullScreenOverlay,
              shouldUseHiddenCameraButtonAppearance && styles.transparentFullScreenOverlay,
            ]}>
            <View style={styles.fullScreenOverlayActions}>
              <Pressable
                accessibilityRole="button"
                onPress={closeFullScreenMode}
                style={({ pressed }) => [
                  styles.overlayActionButton,
                  shouldUseHiddenCameraButtonAppearance && styles.transparentOverlayActionButton,
                  shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
                  pressed && styles.pressedButton,
                ]}>
                <Text
                  style={[
                    styles.overlayActionButtonText,
                    shouldUseHiddenCameraButtonAppearance && styles.transparentOverlayActionButtonText,
                  ]}>
                  Zurück
                </Text>
              </Pressable>
              <Pressable
                accessibilityLabel="Anzeige löschen"
                accessibilityRole="button"
                disabled={controlsAreDisabled}
                onPress={onClearDisplay}
                style={({ pressed }) => [
                  styles.overlayIconButton,
                  shouldUseHiddenCameraButtonAppearance && styles.transparentOverlayActionButton,
                  shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
                  controlsAreDisabled && styles.disabledButton,
                  pressed && !controlsAreDisabled && styles.pressedButton,
                ]}>
                <Text
                  style={[
                    styles.overlayActionButtonText,
                    shouldUseHiddenCameraButtonAppearance && styles.transparentOverlayActionButtonText,
                  ]}>
                  ⌫
                </Text>
              </Pressable>
              <Pressable
                accessibilityRole="button"
                onPress={() => setAreFullScreenControlsVisible(false)}
                style={({ pressed }) => [
                  styles.overlayActionButton,
                  shouldUseHiddenCameraButtonAppearance && styles.transparentOverlayActionButton,
                  shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
                  pressed && styles.pressedButton,
                ]}>
                <Text
                  style={[
                    styles.overlayActionButtonText,
                    shouldUseHiddenCameraButtonAppearance && styles.transparentOverlayActionButtonText,
                  ]}>
                  Bedienung ausblenden
                </Text>
              </Pressable>
            </View>
            <ScrollView
              contentContainerStyle={[
                styles.fullScreenOverlayContent,
                isLandscapeOrientation && styles.fullScreenOverlayContentLandscape,
              ]}>
              <View
                style={[
                  styles.fullScreenControlSurface,
                  shouldUseHiddenCameraButtonAppearance && styles.transparentFullScreenControlSurface,
                ]}>
                <CommandControls
                  cameraButtonOpacity={cameraButtonOpacity}
                  compassGridSize={compassGridSize}
                  controlsAreDisabled={controlsAreDisabled}
                  isLandscapeOrientation={isLandscapeOrientation}
                  onSelectCardSuit={onSelectCardSuit}
                  onSendCardRank={onSendCardRank}
                  onSendCommand={onSendCommand}
                  onSendRawCommand={onSendRawCommand}
                  onSendSymbol={onSendSymbol}
                  rawCommandText={rawCommandText}
                  selectedMode={fullScreenMode}
                  selectedSuitCode={selectedSuitCode}
                  setRawCommandText={setRawCommandText}
                  setSymbolText={setSymbolText}
                  shouldUseHiddenCameraButtonAppearance={shouldUseHiddenCameraButtonAppearance}
                  styles={styles}
                  symbolText={symbolText}
                />
              </View>
            </ScrollView>
          </View>
        ) : null}

        <View style={styles.cameraBottomHud}>
          <Pressable
            accessibilityLabel={isFakeVideoPaused ? 'Video fortsetzen' : 'Video pausieren'}
            accessibilityRole="button"
            onPress={() => setIsFakeVideoPaused((currentValue) => !currentValue)}
            style={({ pressed }) => [styles.fakePauseButton, pressed && styles.pressedButton]}>
            <Text style={styles.fakePauseButtonText}>{isFakeVideoPaused ? '▶' : '⏸'}</Text>
          </Pressable>
          <View style={styles.cameraZoomBadge}>
            <Text style={styles.cameraZoomText}>Zoom {cameraZoomText}</Text>
          </View>
        </View>
      </View>
    </SafeAreaView>
  );
}
