import { SymbolView, type SymbolViewProps } from 'expo-symbols';
import { useCameraPermissions } from 'expo-camera';
import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import {
  Alert,
  BackHandler,
  Image,
  Modal,
  Pressable,
  ScrollView,
  Text,
  useWindowDimensions,
  View,
} from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import { AboutScreen } from '@/features/about/AboutScreen';
import { CameraFullScreenView } from '@/features/camera/CameraFullScreenView';
import { ClientProgram } from '@/features/client/ClientProgram';
import { LogScreen } from '@/features/log/LogScreen';
import { SettingsScreen } from '@/features/settings/SettingsScreen';
import { BlePrompterService } from '@/services/BlePrompterService';
import { loadPersistedSettings, savePersistedSettings } from '@/services/SettingsStorage';
import { getAppMetadataLine } from '@/shared/appMetadata';
import { MAXIMUM_LOG_ENTRIES, menuItemLabels, menuItems } from '@/shared/constants';
import {
  formatRecordingDuration,
  getCameraButtonOpacity,
} from '@/shared/formatters';
import { createStyles, darkColors, lightColors } from '@/shared/styles';
import type {
  BleScanMode,
  CameraButtonAppearance,
  CameraViewStyle,
  ConnectionStatus,
  DiscoveredBleDevice,
  MenuItem,
  ModeLabel,
  RememberedBleDevice,
} from '@/shared/types';

const appIcon = require('../../../assets/images/icon.png');

const menuItemIconNames: Record<MenuItem, SymbolViewProps['name']> = {
  Client: { android: 'photo_camera', ios: 'photo', web: 'photo_camera' },
  Einstellungen: { android: 'settings', ios: 'gearshape', web: 'settings' },
  Log: { android: 'history', ios: 'clock.arrow.circlepath', web: 'history' },
  About: { android: 'info', ios: 'info.circle', web: 'info' },
};

const menuItemFallbackIcons: Record<MenuItem, string> = {
  Client: 'C',
  Einstellungen: 'E',
  Log: 'L',
  About: 'i',
};

/**
 * App-Container: hält globalen Zustand, Menüauswahl und Verbindungen zusammen.
 * Die einzelnen Programmteile sind darunter als austauschbare Feature-Komponenten eingebunden.
 */
export function HomeScreen() {
  const { height: windowHeight, width: windowWidth } = useWindowDimensions();
  const [cameraPermission, requestCameraPermission] = useCameraPermissions();
  const [selectedMenuItem, setSelectedMenuItem] = useState<MenuItem>('Client');
  const [isMenuOpen, setIsMenuOpen] = useState(false);
  const [isDarkModeEnabled, setIsDarkModeEnabled] = useState(false);
  const [shouldCloseButtonViewAfterSend, setShouldCloseButtonViewAfterSend] = useState(true);
  const [selectedCameraViewStyle, setSelectedCameraViewStyle] =
    useState<CameraViewStyle>('android');
  const [selectedCameraButtonAppearance, setSelectedCameraButtonAppearance] =
    useState<CameraButtonAppearance>('normal');
  const [customCameraButtonOpacity, setCustomCameraButtonOpacity] = useState(0.18);
  const [cameraButtonOpacitySliderWidth, setCameraButtonOpacitySliderWidth] = useState(0);
  const [connectionStatus, setConnectionStatus] = useState<ConnectionStatus>('Nicht verbunden');
  const [isConnected, setIsConnected] = useState(false);
  const [isTestModeEnabled, setIsTestModeEnabledState] = useState(false);
  const [isScanning, setIsScanning] = useState(false);
  const [scanMode, setScanMode] = useState<BleScanMode>('filtered');
  const [discoveredDevices, setDiscoveredDevices] = useState<DiscoveredBleDevice[]>([]);
  const [connectedDeviceName, setConnectedDeviceName] = useState('');
  const [selectedMode, setSelectedMode] = useState<ModeLabel>('Pfeile');
  const [fullScreenMode, setFullScreenMode] = useState<ModeLabel | null>(null);
  const [areFullScreenControlsVisible, setAreFullScreenControlsVisible] = useState(false);
  const [recordingStartedAt, setRecordingStartedAt] = useState<number | null>(null);
  const [recordingElapsedSeconds, setRecordingElapsedSeconds] = useState(0);
  const [cameraZoom, setCameraZoom] = useState(0);
  const [isFakeVideoPaused, setIsFakeVideoPaused] = useState(false);
  const [selectedSuitCode, setSelectedSuitCode] = useState('H');
  const [wasCardSuitButtonPressed, setWasCardSuitButtonPressed] = useState(false);
  const [symbolText, setSymbolText] = useState('');
  const [rawCommandText, setRawCommandText] = useState('');
  const [isDisplayInverted, setIsDisplayInvertedState] = useState(false);
  const [isDisplayRotated, setIsDisplayRotatedState] = useState(false);
  const [lastCommand, setLastCommand] = useState('-');
  const [lastResponse, setLastResponse] = useState('-');
  const [messageLog, setMessageLog] = useState<string[]>([]);
  const [rememberedBleDevice, setRememberedBleDevice] = useState<RememberedBleDevice | null>(null);
  const [autoReconnectBleDevice, setAutoReconnectBleDevice] =
    useState<RememberedBleDevice | null>(null);
  const [haveSettingsLoadedFlag, setHaveSettingsLoadedFlag] = useState(false);
  const [scrollViewHeight, setScrollViewHeight] = useState(0);
  const [scrollContentHeight, setScrollContentHeight] = useState(0);
  const [scrollOffsetY, setScrollOffsetY] = useState(0);
  const haveSettingsLoaded = useRef(false);
  const shouldContinueAutoReconnect = useRef(false);
  const hasStartedAutoReconnect = useRef(false);

  const activeColors = isDarkModeEnabled ? darkColors : lightColors;
  const styles = useMemo(() => createStyles(activeColors), [activeColors]);
  const controlsAreDisabled = !isConnected && !isTestModeEnabled;
  const isLandscapeOrientation = windowWidth > windowHeight;
  const compassGridSize = Math.max(240, Math.min(windowWidth - 32, windowHeight - 112, 460));
  const recordingDurationText = formatRecordingDuration(recordingElapsedSeconds);
  const cameraZoomText = `${Math.round(cameraZoom * 100)} %`;
  const cameraButtonOpacity = getCameraButtonOpacity(
    selectedCameraButtonAppearance,
    customCameraButtonOpacity
  );
  const cameraButtonOpacityText = `${Math.round(cameraButtonOpacity * 100)} %`;
  const cameraButtonOpacityTrackFillWidth = cameraButtonOpacitySliderWidth * cameraButtonOpacity;
  const shouldUseHiddenCameraButtonAppearance = selectedCameraButtonAppearance !== 'normal';
  const shouldShowCameraButtonOpacitySlider = selectedCameraButtonAppearance === 'custom';
  const appMetadataLine = useMemo(() => getAppMetadataLine(), []);
  const selectedMenuItemLabel = menuItemLabels[selectedMenuItem];
  const drawerWidth = Math.min(windowWidth * 0.82, 320);
  const hasHiddenScrollContent = scrollContentHeight > scrollViewHeight + 12;
  const isNearScrollBottom = scrollOffsetY + scrollViewHeight >= scrollContentHeight - 24;
  const shouldShowScrollHint = hasHiddenScrollContent && !isNearScrollBottom;

  const addLogEntry = useCallback((message: string) => {
    const timestamp = new Intl.DateTimeFormat('de-DE', {
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
    }).format(new Date());

    setMessageLog((currentEntries) =>
      [`${timestamp} ${message}`, ...currentEntries].slice(0, MAXIMUM_LOG_ENTRIES)
    );
  }, []);

  const bleService = useRef<BlePrompterService | null>(null);

  const serviceEvents = useMemo(
    () => ({
      onConnectionChanged: (nextIsConnected: boolean, nextConnectedDeviceName: string) => {
        setIsConnected(nextIsConnected);
        setConnectedDeviceName(nextConnectedDeviceName);
      },
      onDevicesChanged: setDiscoveredDevices,
      onLog: addLogEntry,
      onResponse: setLastResponse,
      onScanningChanged: setIsScanning,
      onStatusChanged: setConnectionStatus,
    }),
    [addLogEntry]
  );

  if (bleService.current === null) {
    bleService.current = new BlePrompterService(serviceEvents);
  }

  useEffect(() => {
    bleService.current?.updateEvents(serviceEvents);
  }, [serviceEvents]);

  useEffect(() => {
    let shouldApplyLoadedSettings = true;

    async function loadSettings() {
      try {
        const persistedSettings = await loadPersistedSettings();

        if (!shouldApplyLoadedSettings) {
          return;
        }

        if (persistedSettings?.isDarkModeEnabled !== undefined) {
          setIsDarkModeEnabled(persistedSettings.isDarkModeEnabled);
        }

        if (persistedSettings?.isDisplayInverted !== undefined) {
          setIsDisplayInvertedState(persistedSettings.isDisplayInverted);
        }

        if (persistedSettings?.isDisplayRotated !== undefined) {
          setIsDisplayRotatedState(persistedSettings.isDisplayRotated);
        }

        if (persistedSettings?.shouldCloseButtonViewAfterSend !== undefined) {
          setShouldCloseButtonViewAfterSend(persistedSettings.shouldCloseButtonViewAfterSend);
        }

        if (persistedSettings?.selectedCameraViewStyle) {
          setSelectedCameraViewStyle(persistedSettings.selectedCameraViewStyle);
        }

        if (persistedSettings?.selectedCameraButtonAppearance) {
          setSelectedCameraButtonAppearance(persistedSettings.selectedCameraButtonAppearance);
        }

        if (persistedSettings?.customCameraButtonOpacity !== undefined) {
          setCustomCameraButtonOpacity(persistedSettings.customCameraButtonOpacity);
        }

        if (persistedSettings?.rememberedBleDevice !== undefined) {
          setRememberedBleDevice(persistedSettings.rememberedBleDevice);
        }
      } catch (error) {
        console.warn('Einstellungen konnten nicht geladen werden.', error);
      } finally {
        if (shouldApplyLoadedSettings) {
          haveSettingsLoaded.current = true;
          setHaveSettingsLoadedFlag(true);
        }
      }
    }

    loadSettings();

    return () => {
      shouldApplyLoadedSettings = false;
    };
  }, []);

  useEffect(() => {
    if (!haveSettingsLoaded.current) {
      return;
    }

    savePersistedSettings({
      customCameraButtonOpacity,
      isDarkModeEnabled,
      isDisplayInverted,
      isDisplayRotated,
      rememberedBleDevice,
      selectedCameraButtonAppearance,
      selectedCameraViewStyle,
      shouldCloseButtonViewAfterSend,
    }).catch((error) => {
      console.warn('Einstellungen konnten nicht gespeichert werden.', error);
    });
  }, [
    customCameraButtonOpacity,
    isDarkModeEnabled,
    isDisplayInverted,
    isDisplayRotated,
    rememberedBleDevice,
    selectedCameraButtonAppearance,
    selectedCameraViewStyle,
    shouldCloseButtonViewAfterSend,
  ]);

  useEffect(() => {
    if (
      !haveSettingsLoaded.current ||
      hasStartedAutoReconnect.current ||
      !rememberedBleDevice ||
      isConnected
    ) {
      return;
    }

    hasStartedAutoReconnect.current = true;
    shouldContinueAutoReconnect.current = true;
    setAutoReconnectBleDevice(rememberedBleDevice);

    bleService.current
      ?.reconnectToRememberedDevice(
        rememberedBleDevice,
        () => shouldContinueAutoReconnect.current
      )
      .then((wasConnected) => {
        if (!wasConnected) {
          setSelectedMenuItem('Client');
        }
      })
      .finally(() => {
        shouldContinueAutoReconnect.current = false;
        setAutoReconnectBleDevice(null);
      });
  }, [isConnected, rememberedBleDevice]);

  const setTestModeEnabled = useCallback(
    async (nextValue: boolean) => {
      setIsTestModeEnabledState(nextValue);

      if (nextValue) {
        await bleService.current?.disconnect();
        setDiscoveredDevices([]);
        setIsConnected(false);
        setConnectedDeviceName('');
        setConnectionStatus('Testmodus');
        setLastResponse('Testmodus aktiv. Bluetooth wird nicht verwendet.');
        addLogEntry('Testmodus aktiviert.');
        return;
      }

      bleService.current?.resetBluetoothState('Nicht verbunden');
      setLastResponse('Testmodus beendet.');
      addLogEntry('Testmodus beendet.');
    },
    [addLogEntry]
  );

  const scanForDevices = useCallback(async (mode: BleScanMode) => {
    setScanMode(mode);
    await bleService.current?.scanForDevices(mode);
  }, []);

  const connectToDevice = useCallback(async (discoveredDevice: DiscoveredBleDevice) => {
    const wasConnected = await bleService.current?.connectToDevice(discoveredDevice);

    if (wasConnected) {
      setRememberedBleDevice({ id: discoveredDevice.id, name: discoveredDevice.name });
    }
  }, []);

  const cancelAutoReconnect = useCallback(() => {
    shouldContinueAutoReconnect.current = false;
    bleService.current?.resetBluetoothState('Nicht verbunden');
    setAutoReconnectBleDevice(null);
    setSelectedMenuItem('Client');
    addLogEntry('Automatische Verbindung abgebrochen.');
  }, [addLogEntry]);

  const disconnect = useCallback(async () => {
    await bleService.current?.disconnect();
  }, []);

  const sendCommand = useCallback(
    async (command: string) => {
      setLastCommand(command);

      if (isTestModeEnabled) {
        setLastResponse(`Testmodus: ${command}`);
        addLogEntry(`Testmodus gesendet: ${command}`);
        return true;
      }

      if (!isConnected) {
        const message = 'BlePrompter ist nicht verbunden.';
        setLastResponse(message);
        addLogEntry(message);
        return false;
      }

      return Boolean(await bleService.current?.sendCommand(command));
    },
    [addLogEntry, isConnected, isTestModeEnabled]
  );

  const closeButtonViewAfterSuccessfulSend = useCallback(() => {
    if (shouldCloseButtonViewAfterSend) {
      setAreFullScreenControlsVisible(false);
    }
  }, [shouldCloseButtonViewAfterSend]);

  const sendCommandAndCloseButtonView = useCallback(
    async (command: string) => {
      const wasSent = await sendCommand(command);

      if (wasSent) {
        closeButtonViewAfterSuccessfulSend();
      }
    },
    [closeButtonViewAfterSuccessfulSend, sendCommand]
  );

  const clearDisplay = useCallback(async () => {
    const wasCleared = await sendCommand('CL');

    if (wasCleared) {
      await sendCommand('SLEEP DISPLAY');
    }
  }, [sendCommand]);

  const selectCardSuit = useCallback((suitCode: string) => {
    setSelectedSuitCode(suitCode);
    setWasCardSuitButtonPressed(true);
  }, []);

  const sendCardRank = useCallback(
    async (rankCode: string) => {
      const wasSent = await sendCommand(`C${selectedSuitCode}${rankCode}`);

      if (!wasSent) {
        return;
      }

      if (wasCardSuitButtonPressed) {
        closeButtonViewAfterSuccessfulSend();
      }

      setWasCardSuitButtonPressed(false);
    },
    [closeButtonViewAfterSuccessfulSend, selectedSuitCode, sendCommand, wasCardSuitButtonPressed]
  );

  const setDisplayInverted = useCallback(
    (nextValue: boolean) => {
      setIsDisplayInvertedState(nextValue);

      if (isConnected) {
        void sendCommand(nextValue ? 'I1' : 'I0');
      }
    },
    [isConnected, sendCommand]
  );

  const setDisplayRotated = useCallback(
    (nextValue: boolean) => {
      setIsDisplayRotatedState(nextValue);

      if (isConnected) {
        void sendCommand(nextValue ? 'U1' : 'U0');
      }
    },
    [isConnected, sendCommand]
  );

  const previousIsConnected = useRef(false);

  useEffect(() => {
    if (!haveSettingsLoadedFlag) {
      return;
    }

    const hasJustConnected = isConnected && !previousIsConnected.current;
    previousIsConnected.current = isConnected;

    if (!hasJustConnected) {
      return;
    }

    async function synchronizeDisplaySettings() {
      await sendCommand(isDisplayInverted ? 'I1' : 'I0');
      await sendCommand(isDisplayRotated ? 'U1' : 'U0');
    }

    void synchronizeDisplaySettings();
  }, [haveSettingsLoadedFlag, isConnected, isDisplayInverted, isDisplayRotated, sendCommand]);

  const sendSymbol = useCallback(async () => {
    const normalizedSymbolText = symbolText.trim().slice(0, 2).toUpperCase();

    if (!normalizedSymbolText) {
      const message = 'Symbol fehlt.';
      setLastResponse(message);
      addLogEntry(message);
      return;
    }

    const wasSent = await sendCommand(`SYMBOL ${normalizedSymbolText}`);

    if (wasSent) {
      closeButtonViewAfterSuccessfulSend();
    }
  }, [addLogEntry, closeButtonViewAfterSuccessfulSend, sendCommand, symbolText]);

  const sendRawCommand = useCallback(async () => {
    if (rawCommandText.length === 0) {
      const message = 'Raw-Befehl fehlt.';
      setLastResponse(message);
      addLogEntry(message);
      return;
    }

    const wasSent = await sendCommand(rawCommandText);

    if (wasSent) {
      closeButtonViewAfterSuccessfulSend();
    }
  }, [addLogEntry, closeButtonViewAfterSuccessfulSend, rawCommandText, sendCommand]);

  const openFullScreenMode = (modeLabel: ModeLabel) => {
    setSelectedMode(modeLabel);
    setFullScreenMode(modeLabel);
    setAreFullScreenControlsVisible(false);
    setWasCardSuitButtonPressed(false);
    setRecordingStartedAt(Date.now());
    setRecordingElapsedSeconds(0);
    setIsFakeVideoPaused(false);
  };

  const closeFullScreenMode = () => {
    setFullScreenMode(null);
    setAreFullScreenControlsVisible(false);
    setRecordingStartedAt(null);
    setRecordingElapsedSeconds(0);
  };

  useEffect(
    () => () => {
      bleService.current?.destroy();
    },
    []
  );

  useEffect(() => {
    if (!fullScreenMode || !recordingStartedAt) {
      return;
    }

    const updateRecordingDuration = () => {
      setRecordingElapsedSeconds(Math.floor((Date.now() - recordingStartedAt) / 1000));
    };

    updateRecordingDuration();
    const intervalId = setInterval(updateRecordingDuration, 1000);

    return () => clearInterval(intervalId);
  }, [fullScreenMode, recordingStartedAt]);

  useEffect(() => {
    const backSubscription = BackHandler.addEventListener('hardwareBackPress', () => {
      if (fullScreenMode) {
        if (areFullScreenControlsVisible) {
          setAreFullScreenControlsVisible(false);
          return true;
        }

        closeFullScreenMode();
        return true;
      }

      if (isMenuOpen) {
        setIsMenuOpen(false);
        return true;
      }

      if (selectedMenuItem !== 'Client') {
        setSelectedMenuItem('Client');
        return true;
      }

      Alert.alert('Anwendung beenden?', 'Möchtest du die Anwendung wirklich verlassen?', [
        { text: 'Abbrechen', style: 'cancel' },
        { text: 'Beenden', style: 'destructive', onPress: () => BackHandler.exitApp() },
      ]);

      return true;
    });

    return () => backSubscription.remove();
  }, [areFullScreenControlsVisible, fullScreenMode, isMenuOpen, selectedMenuItem]);

  if (fullScreenMode) {
    return (
      <CameraFullScreenView
        areFullScreenControlsVisible={areFullScreenControlsVisible}
        cameraButtonOpacity={cameraButtonOpacity}
        cameraPermission={cameraPermission}
        cameraZoom={cameraZoom}
        cameraZoomText={cameraZoomText}
        closeFullScreenMode={closeFullScreenMode}
        compassGridSize={compassGridSize}
        controlsAreDisabled={controlsAreDisabled}
        fullScreenMode={fullScreenMode}
        isFakeVideoPaused={isFakeVideoPaused}
        isLandscapeOrientation={isLandscapeOrientation}
        onClearDisplay={clearDisplay}
        onSelectCardSuit={selectCardSuit}
        onSendCardRank={sendCardRank}
        onSendCommand={sendCommandAndCloseButtonView}
        onSendRawCommand={sendRawCommand}
        onSendSymbol={sendSymbol}
        rawCommandText={rawCommandText}
        recordingDurationText={recordingDurationText}
        requestCameraPermission={requestCameraPermission}
        selectedCameraViewStyle={selectedCameraViewStyle}
        selectedSuitCode={selectedSuitCode}
        setAreFullScreenControlsVisible={setAreFullScreenControlsVisible}
        setCameraZoom={setCameraZoom}
        setIsFakeVideoPaused={setIsFakeVideoPaused}
        setRawCommandText={setRawCommandText}
        setSymbolText={setSymbolText}
        shouldUseHiddenCameraButtonAppearance={shouldUseHiddenCameraButtonAppearance}
        styles={styles}
        symbolText={symbolText}
      />
    );
  }

  return (
    <SafeAreaView style={styles.safeArea}>
      <Modal
        animationType="fade"
        onRequestClose={() => setIsMenuOpen(false)}
        transparent
        visible={isMenuOpen}>
        <View style={styles.drawerBackdrop}>
          <View style={[styles.drawerPanel, { width: drawerWidth }]}>
            <View style={styles.drawerHeader}>
              <Image source={appIcon} style={styles.titleIcon} />
              <View style={styles.drawerHeaderText}>
                <Text style={styles.drawerHeaderTitle}>Menü</Text>
                <Text style={styles.drawerHeaderSubtitle}>{appMetadataLine}</Text>
              </View>
            </View>
            <View>
              {menuItems.map((menuItem) => {
                const isActiveMenuItem = selectedMenuItem === menuItem;

                return (
                  <Pressable
                    accessibilityRole="menuitem"
                    key={menuItem}
                    onPress={() => {
                      setSelectedMenuItem(menuItem);
                      setIsMenuOpen(false);
                    }}
                    style={({ pressed }) => [
                      styles.drawerItem,
                      isActiveMenuItem && styles.activeDrawerItem,
                      pressed && styles.pressedButton,
                    ]}>
                    <View
                      style={[
                        styles.drawerActiveIndicator,
                        !isActiveMenuItem && styles.inactiveDrawerActiveIndicator,
                      ]}
                    />
                    <SymbolView
                      fallback={
                        <Text
                          style={[
                            styles.drawerItemFallbackIcon,
                            isActiveMenuItem && styles.activeDrawerItemText,
                          ]}>
                          {menuItemFallbackIcons[menuItem]}
                        </Text>
                      }
                      name={menuItemIconNames[menuItem]}
                      size={22}
                      tintColor={
                        isActiveMenuItem ? activeColors.accentStrong : activeColors.warm
                      }
                      type="monochrome"
                      weight="semibold"
                    />
                    <Text
                      style={[
                        styles.drawerItemText,
                        isActiveMenuItem && styles.activeDrawerItemText,
                      ]}>
                      {menuItemLabels[menuItem]}
                    </Text>
                  </Pressable>
                );
              })}
            </View>
          </View>
          <Pressable
            accessibilityLabel="Menü schließen"
            accessibilityRole="button"
            onPress={() => setIsMenuOpen(false)}
            style={styles.drawerDismissArea}
          />
        </View>
      </Modal>
      <Modal animationType="fade" transparent visible={Boolean(autoReconnectBleDevice)}>
        <View style={styles.modalBackdrop}>
          <View style={styles.modalPanel}>
            <Text style={styles.modalTitle}>Verbindung wird wiederhergestellt</Text>
            <Text style={styles.modalText}>
              Suche zuletzt genutztes Gerät: {autoReconnectBleDevice?.name}
            </Text>
            <Pressable
              accessibilityRole="button"
              onPress={cancelAutoReconnect}
              style={[styles.button, styles.dangerButton]}>
              <Text style={[styles.buttonText, styles.dangerButtonText]}>
                Verbindungsversuch abbrechen
              </Text>
            </Pressable>
          </View>
        </View>
      </Modal>
      <ScrollView
        contentContainerStyle={styles.container}
        indicatorStyle={isDarkModeEnabled ? 'white' : 'black'}
        onContentSizeChange={(_, contentHeight) => setScrollContentHeight(contentHeight)}
        onLayout={(event) => setScrollViewHeight(event.nativeEvent.layout.height)}
        onScroll={(event) => setScrollOffsetY(event.nativeEvent.contentOffset.y)}
        persistentScrollbar
        scrollEventThrottle={16}
        showsVerticalScrollIndicator>
        <View style={styles.header}>
          <View style={styles.headerTopRow}>
            <Pressable
              accessibilityLabel="Menü"
              accessibilityRole="button"
              onPress={() => setIsMenuOpen((currentValue) => !currentValue)}
              style={({ pressed }) => [styles.menuButton, pressed && styles.pressedButton]}>
              <Text style={styles.menuButtonText}>☰</Text>
            </Pressable>
            <View style={styles.titleBlock}>
              <View style={styles.titleRow}>
                <Image source={appIcon} style={styles.titleIcon} />
                <Text style={styles.title}>Nasreddins Simple Peek Client 2</Text>
              </View>
              <Text style={styles.eyebrow}>{appMetadataLine}</Text>
            </View>
          </View>
          <View style={styles.headerStatusRow}>
            <View
              style={[
                styles.connectionState,
                (isConnected || isTestModeEnabled) && styles.connectedState,
              ]}>
              <View style={[styles.stateDot, (isConnected || isTestModeEnabled) && styles.connectedStateDot]} />
              <Text
                style={[
                  styles.connectionStateText,
                  (isConnected || isTestModeEnabled) && styles.connectedStateText,
                ]}>
                {connectionStatus}
              </Text>
            </View>
          </View>
        </View>

        <Text style={styles.screenHeading}>{selectedMenuItemLabel}</Text>

        {selectedMenuItem === 'Client' ? (
          <ClientProgram
            connectedDeviceName={connectedDeviceName}
            connectionStatus={connectionStatus}
            controlsAreDisabled={controlsAreDisabled}
            discoveredDevices={discoveredDevices}
            isConnected={isConnected}
            isScanning={isScanning}
            isTestModeEnabled={isTestModeEnabled}
            lastCommand={lastCommand}
            lastResponse={lastResponse}
            onClearDisplay={clearDisplay}
            onConnectToDevice={connectToDevice}
            onDisconnect={disconnect}
            onOpenFullScreenMode={openFullScreenMode}
            onScanForDevices={scanForDevices}
            scanMode={scanMode}
            selectedMode={selectedMode}
            setTestModeEnabled={setTestModeEnabled}
            styles={styles}
          />
        ) : null}

        {selectedMenuItem === 'Einstellungen' ? (
          <SettingsScreen
            cameraButtonOpacitySliderWidth={cameraButtonOpacitySliderWidth}
            cameraButtonOpacityText={cameraButtonOpacityText}
            cameraButtonOpacityTrackFillWidth={cameraButtonOpacityTrackFillWidth}
            customCameraButtonOpacity={customCameraButtonOpacity}
            isDarkModeEnabled={isDarkModeEnabled}
            isDisplayInverted={isDisplayInverted}
            isDisplayRotated={isDisplayRotated}
            selectedCameraButtonAppearance={selectedCameraButtonAppearance}
            selectedCameraViewStyle={selectedCameraViewStyle}
            setCameraButtonOpacitySliderWidth={setCameraButtonOpacitySliderWidth}
            setCustomCameraButtonOpacity={setCustomCameraButtonOpacity}
            setDisplayInverted={setDisplayInverted}
            setDisplayRotated={setDisplayRotated}
            setIsDarkModeEnabled={setIsDarkModeEnabled}
            setSelectedCameraButtonAppearance={setSelectedCameraButtonAppearance}
            setSelectedCameraViewStyle={setSelectedCameraViewStyle}
            setShouldCloseButtonViewAfterSend={setShouldCloseButtonViewAfterSend}
            shouldCloseButtonViewAfterSend={shouldCloseButtonViewAfterSend}
            shouldShowCameraButtonOpacitySlider={shouldShowCameraButtonOpacitySlider}
            styles={styles}
          />
        ) : null}

        {selectedMenuItem === 'Log' ? <LogScreen messageLog={messageLog} styles={styles} /> : null}

        {selectedMenuItem === 'About' ? <AboutScreen styles={styles} /> : null}
      </ScrollView>
      {shouldShowScrollHint ? (
        <View pointerEvents="none" style={styles.scrollHint}>
          <Text style={styles.scrollHintText}>Weiter nach unten ↓</Text>
        </View>
      ) : null}
    </SafeAreaView>
  );
}
