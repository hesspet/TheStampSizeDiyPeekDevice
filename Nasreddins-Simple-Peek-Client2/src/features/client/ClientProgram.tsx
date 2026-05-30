import { Pressable, Switch, Text, View } from 'react-native';

import { CommandButton } from '@/features/commands/CommandButton';
import { getDeviceDisplayName } from '@/shared/formatters';
import type { AppStyles } from '@/shared/styles';
import type { BleScanMode, DiscoveredBleDevice, ModeLabel } from '@/shared/types';
import { modeLabels } from '@/shared/constants';

type ClientProgramProps = {
  connectedDeviceName: string;
  connectionStatus: string;
  controlsAreDisabled: boolean;
  discoveredDevices: DiscoveredBleDevice[];
  isConnected: boolean;
  isScanning: boolean;
  isTestModeEnabled: boolean;
  lastCommand: string;
  lastResponse: string;
  onClearDisplay: () => void | Promise<unknown>;
  onConnectToDevice: (device: DiscoveredBleDevice) => void | Promise<unknown>;
  onDisconnect: () => void | Promise<unknown>;
  onOpenFullScreenMode: (modeLabel: ModeLabel) => void;
  onScanForDevices: (mode: BleScanMode) => void | Promise<unknown>;
  scanMode: BleScanMode;
  selectedMode: ModeLabel;
  setTestModeEnabled: (nextValue: boolean) => void | Promise<void>;
  styles: AppStyles;
};

/**
 * Hauptprogramm für die BlePrompter-Steuerung.
 * Dieser Menüpunkt ist vom App-Container getrennt und kann später neben weiteren Programmen bestehen.
 */
export function ClientProgram({
  connectedDeviceName,
  controlsAreDisabled,
  discoveredDevices,
  isConnected,
  isScanning,
  isTestModeEnabled,
  lastCommand,
  lastResponse,
  onClearDisplay,
  onConnectToDevice,
  onDisconnect,
  onOpenFullScreenMode,
  onScanForDevices,
  scanMode,
  selectedMode,
  setTestModeEnabled,
  styles,
}: ClientProgramProps) {
  const shouldShowConnectionButtons = !isConnected && !isTestModeEnabled;
  const shouldShowDisconnectButton = isConnected && !isTestModeEnabled;
  const shouldShowDevicePanel =
    shouldShowConnectionButtons && (isScanning || discoveredDevices.length > 0);
  const shouldShowExtendedToolbarControls = !isConnected;

  return (
    <>
      <View style={styles.toolbar}>
        {shouldShowConnectionButtons ? (
          <CommandButton
            disabled={isScanning}
            isPrimary
            label="Verbinden"
            onPress={() => onScanForDevices('filtered')}
            styles={styles}
          />
        ) : null}
        <View style={styles.toolbarActionRow}>
          <View style={styles.toolbarActionCell}>
            <CommandButton
              disabled={controlsAreDisabled}
              icon="⌫"
              label="Anzeige löschen"
              onPress={onClearDisplay}
              styles={styles}
            />
          </View>
          {shouldShowDisconnectButton ? (
            <View style={styles.toolbarActionCell}>
              <CommandButton label="Trennen" onPress={onDisconnect} styles={styles} />
            </View>
          ) : null}
        </View>
        {shouldShowExtendedToolbarControls ? (
          <View style={styles.toggleControl}>
            <Text style={styles.toggleText}>Testmodus</Text>
            <Switch onValueChange={setTestModeEnabled} value={isTestModeEnabled} />
          </View>
        ) : null}
      </View>

      {shouldShowDevicePanel ? (
        <View style={styles.devicePanel}>
          <Text style={styles.sectionTitle}>{isScanning ? 'Gefundene Geräte' : 'Scan beendet'}</Text>
          {discoveredDevices.length === 0 ? (
            <Text style={styles.mutedText}>Suche läuft. BlePrompter bitte eingeschaltet lassen.</Text>
          ) : (
            discoveredDevices.map((device) => (
              <Pressable
                accessibilityRole="button"
                key={device.id}
                onPress={() => onConnectToDevice(device)}
                style={styles.deviceButton}>
                <Text style={styles.deviceButtonText}>{getDeviceDisplayName(device)}</Text>
              </Pressable>
            ))
          )}
          <Text style={styles.mutedText}>
            {scanMode === 'filtered'
              ? 'Gefiltert nach BlePrompter und Nordic UART.'
              : 'Alle sichtbaren BLE-Geräte werden angezeigt.'}
          </Text>
        </View>
      ) : null}

      {connectedDeviceName ? (
        <Text style={styles.connectedDeviceText}>Verbunden mit {connectedDeviceName}</Text>
      ) : null}

      <View style={styles.modeTabs}>
        {modeLabels.map((modeLabel) => (
          <Pressable
            accessibilityRole="button"
            accessibilityState={{ selected: selectedMode === modeLabel }}
            disabled={controlsAreDisabled}
            key={modeLabel}
            onPress={() => onOpenFullScreenMode(modeLabel)}
            style={[
              styles.modeTab,
              selectedMode === modeLabel && styles.activeModeTab,
              controlsAreDisabled && styles.disabledButton,
            ]}>
            <Text style={[styles.modeTabText, selectedMode === modeLabel && styles.activeModeTabText]}>
              {modeLabel}
            </Text>
          </Pressable>
        ))}
      </View>

      <View style={styles.statusPanel}>
        <View style={styles.statusLine}>
          <Text style={styles.statusLabel}>Letzter Befehl</Text>
          <Text numberOfLines={1} style={styles.statusValue}>
            {lastCommand}
          </Text>
        </View>
        <View style={styles.statusLine}>
          <Text style={styles.statusLabel}>Antwort</Text>
          <Text numberOfLines={2} style={styles.statusValue}>
            {lastResponse}
          </Text>
        </View>
      </View>
    </>
  );
}
