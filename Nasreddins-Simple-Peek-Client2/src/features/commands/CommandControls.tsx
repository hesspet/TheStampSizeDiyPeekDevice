import { Text, TextInput, View, Pressable } from 'react-native';

import { directionCommands, espSymbolCommands, rankOptions, suitOptions } from '@/shared/constants';
import type { AppStyles } from '@/shared/styles';
import type { ModeLabel } from '@/shared/types';

import { CommandButton } from './CommandButton';

type CommandControlsProps = {
  cameraButtonOpacity: number;
  compassGridSize: number;
  controlsAreDisabled: boolean;
  isLandscapeOrientation: boolean;
  onSendCardRank: (rankCode: string) => void | Promise<unknown>;
  onSendCommand: (command: string) => void | Promise<unknown>;
  onSendRawCommand: () => void | Promise<unknown>;
  onSendSymbol: () => void | Promise<unknown>;
  onSelectCardSuit: (suitCode: string) => void;
  rawCommandText: string;
  selectedMode: ModeLabel;
  selectedSuitCode: string;
  setRawCommandText: (rawCommandText: string) => void;
  setSymbolText: (symbolText: string) => void;
  shouldUseHiddenCameraButtonAppearance: boolean;
  styles: AppStyles;
  symbolText: string;
};

/**
 * Zeigt die unabhängigen Bedienmodule.
 * Die Komponente kennt nur UI-Zustand und sendet Befehle über Callback-Funktionen.
 */
export function CommandControls({
  cameraButtonOpacity,
  compassGridSize,
  controlsAreDisabled,
  isLandscapeOrientation,
  onSendCardRank,
  onSendCommand,
  onSendRawCommand,
  onSendSymbol,
  onSelectCardSuit,
  rawCommandText,
  selectedMode,
  selectedSuitCode,
  setRawCommandText,
  setSymbolText,
  shouldUseHiddenCameraButtonAppearance,
  styles,
  symbolText,
}: CommandControlsProps) {
  const sharedButtonProps = {
    cameraButtonOpacity,
    isCameraOverlay: true,
    shouldUseHiddenCameraButtonAppearance,
    styles,
  };

  if (selectedMode === 'Pfeile') {
    return (
      <View style={[styles.compassGrid, { width: compassGridSize }]}>
        {directionCommands.slice(0, 3).map((direction) => (
          <View key={direction.command} style={styles.compassCell}>
            <CommandButton
              {...sharedButtonProps}
              disabled={controlsAreDisabled}
              icon={direction.arrow}
              label={direction.label}
              onPress={() => onSendCommand(direction.command)}
            />
          </View>
        ))}
        <View style={styles.compassCell}>
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            icon="←"
            label="W"
            onPress={() => onSendCommand('AW')}
          />
        </View>
        <View
          style={[
            styles.compassCenter,
            shouldUseHiddenCameraButtonAppearance && styles.transparentCameraButton,
            shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
          ]}>
          <Text
            style={[
              styles.compassCenterText,
              shouldUseHiddenCameraButtonAppearance && styles.transparentCameraButtonText,
            ]}>
            +
          </Text>
        </View>
        <View style={styles.compassCell}>
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            icon="→"
            label="O"
            onPress={() => onSendCommand('AE')}
          />
        </View>
        {directionCommands.slice(5).map((direction) => (
          <View key={direction.command} style={styles.compassCell}>
            <CommandButton
              {...sharedButtonProps}
              disabled={controlsAreDisabled}
              icon={direction.arrow}
              label={direction.label}
              onPress={() => onSendCommand(direction.command)}
            />
          </View>
        ))}
      </View>
    );
  }

  if (selectedMode === 'Karten') {
    return (
      <View
        style={[
          styles.panel,
          shouldUseHiddenCameraButtonAppearance && styles.transparentCameraPanel,
          isLandscapeOrientation && styles.cardPanelLandscape,
        ]}>
        <View style={[styles.suitGrid, isLandscapeOrientation && styles.suitGridLandscape]}>
          {suitOptions.map((suit) => (
            <Pressable
              accessibilityRole="button"
              accessibilityState={{ selected: selectedSuitCode === suit.code }}
              disabled={controlsAreDisabled}
              key={suit.code}
              onPress={() => onSelectCardSuit(suit.code)}
              style={[
                styles.suitButton,
                isLandscapeOrientation && styles.suitButtonLandscape,
                selectedSuitCode === suit.code && styles.selectedButton,
                shouldUseHiddenCameraButtonAppearance && styles.transparentCameraButton,
                shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
                controlsAreDisabled && styles.disabledButton,
              ]}>
              <Text
                style={[
                  styles.suitMark,
                  suit.isRed && styles.redSuitMark,
                  shouldUseHiddenCameraButtonAppearance && styles.transparentCameraSuitMark,
                ]}>
                {suit.mark}
              </Text>
              <Text
                style={[
                  styles.buttonText,
                  shouldUseHiddenCameraButtonAppearance && styles.transparentCameraButtonText,
                ]}>
                {suit.label}
              </Text>
            </Pressable>
          ))}
        </View>
        <View style={[styles.rankGrid, isLandscapeOrientation && styles.rankGridLandscape]}>
          {rankOptions.map((rank) => (
            <View key={rank.code} style={[styles.rankCell, isLandscapeOrientation && styles.rankCellLandscape]}>
              <CommandButton
                {...sharedButtonProps}
                disabled={controlsAreDisabled}
                label={rank.label}
                onPress={() => onSendCardRank(rank.code)}
              />
            </View>
          ))}
        </View>
        <View style={[styles.jokerRow, isLandscapeOrientation && styles.jokerRowLandscape]}>
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            label="Joker 1"
            onPress={() => onSendCommand('CJ1')}
          />
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            label="Joker 2"
            onPress={() => onSendCommand('CJ2')}
          />
        </View>
      </View>
    );
  }

  if (selectedMode === 'Symbole') {
    return (
      <View style={styles.symbolPanel}>
        <Text style={styles.inputLabel}>Symbol</Text>
        <View style={[styles.symbolInputRow, isLandscapeOrientation && styles.symbolInputRowLandscape]}>
          <TextInput
            autoCapitalize="characters"
            editable={!controlsAreDisabled}
            maxLength={2}
            onChangeText={(nextText) => setSymbolText(nextText.slice(0, 2).toUpperCase())}
            style={[
              styles.symbolInput,
              isLandscapeOrientation && styles.symbolInputLandscape,
              shouldUseHiddenCameraButtonAppearance && styles.transparentCameraInput,
              shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
              controlsAreDisabled && styles.disabledInput,
            ]}
            value={symbolText}
          />
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            isPrimary
            label="Senden"
            onPress={onSendSymbol}
          />
        </View>
      </View>
    );
  }

  if (selectedMode === 'Raw') {
    return (
      <View style={styles.symbolPanel}>
        <Text style={styles.inputLabel}>Raw-Befehl</Text>
        <View style={[styles.symbolInputRow, isLandscapeOrientation && styles.symbolInputRowLandscape]}>
          <TextInput
            autoCapitalize="none"
            autoCorrect={false}
            editable={!controlsAreDisabled}
            onChangeText={setRawCommandText}
            onSubmitEditing={onSendRawCommand}
            returnKeyType="send"
            style={[
              styles.symbolInput,
              styles.rawCommandInput,
              isLandscapeOrientation && styles.symbolInputLandscape,
              shouldUseHiddenCameraButtonAppearance && styles.transparentCameraInput,
              shouldUseHiddenCameraButtonAppearance && { opacity: cameraButtonOpacity },
              controlsAreDisabled && styles.disabledInput,
            ]}
            value={rawCommandText}
          />
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            isPrimary
            label="Senden"
            onPress={onSendRawCommand}
          />
        </View>
      </View>
    );
  }

  return (
    <View style={styles.espGrid}>
      {espSymbolCommands.map((espSymbol) => (
        <View key={espSymbol.command} style={[styles.espCell, isLandscapeOrientation && styles.espCellLandscape]}>
          <CommandButton
            {...sharedButtonProps}
            disabled={controlsAreDisabled}
            icon={espSymbol.symbol}
            label={espSymbol.label}
            onPress={() => onSendCommand(espSymbol.command)}
          />
        </View>
      ))}
    </View>
  );
}
