import Constants from 'expo-constants';
import { Platform } from 'react-native';

type ExpoUpdatesManifest = {
  createdAt?: string;
};

function formatGermanDateTime(value: Date) {
  return new Intl.DateTimeFormat('de-DE', {
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    month: '2-digit',
    year: 'numeric',
  }).format(value);
}

function getUpdateDateText() {
  const manifestCreatedAt = (Constants.manifest2 as ExpoUpdatesManifest | null)?.createdAt;
  const updateDate = typeof manifestCreatedAt === 'string' ? new Date(manifestCreatedAt) : null;

  if (!updateDate || Number.isNaN(updateDate.getTime())) {
    return 'Update unbekannt';
  }

  return `Update ${formatGermanDateTime(updateDate)}`;
}

function getBuildNumberText() {
  const nativeBuildVersion = Constants.nativeBuildVersion;
  const configuredBuildNumber =
    Platform.OS === 'ios'
      ? Constants.expoConfig?.ios?.buildNumber
      : Constants.expoConfig?.android?.versionCode;
  const buildNumber = nativeBuildVersion ?? configuredBuildNumber;

  return buildNumber ? `Build ${buildNumber}` : 'Build unbekannt';
}

export function getAppMetadataLine() {
  const appVersion = Constants.expoConfig?.version ?? 'Version unbekannt';

  return `Version ${appVersion} · ${getBuildNumberText()} · ${getUpdateDateText()}`;
}
