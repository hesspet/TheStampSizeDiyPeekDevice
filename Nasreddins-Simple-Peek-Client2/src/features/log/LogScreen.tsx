import { Text, View } from 'react-native';

import type { AppStyles } from '@/shared/styles';

type LogScreenProps = {
  messageLog: string[];
  styles: AppStyles;
};

/**
 * Zeigt technische Laufzeitmeldungen getrennt vom Hauptprogramm.
 */
export function LogScreen({ messageLog, styles }: LogScreenProps) {
  return (
    <View style={styles.panel}>
      <Text style={styles.sectionTitle}>Log</Text>
      {messageLog.length === 0 ? (
        <Text style={styles.mutedText}>Noch keine Logeinträge vorhanden.</Text>
      ) : (
        <View style={styles.statusPanel}>
          {messageLog.map((entry, index) => (
            <Text key={`${entry}-${index}`} style={styles.logEntry}>
              {entry}
            </Text>
          ))}
        </View>
      )}
    </View>
  );
}
