export function encodeTextAsBase64(text: string) {
  const bytes = encodeURIComponent(text).replace(/%([0-9A-F]{2})/g, (_match, hexValue: string) =>
    String.fromCharCode(Number.parseInt(hexValue, 16))
  );

  return btoa(bytes);
}

export function decodeBase64AsText(base64Text: string) {
  try {
    const binaryText = atob(base64Text);
    const percentEncodedText = Array.from(binaryText)
      .map((character) => `%${character.charCodeAt(0).toString(16).padStart(2, '0')}`)
      .join('');

    return decodeURIComponent(percentEncodedText);
  } catch {
    return base64Text;
  }
}
