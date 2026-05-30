import { Asset } from 'expo-asset';
import { useEffect, useMemo, useState } from 'react';
import { Image, Text, View } from 'react-native';

import type { AppStyles } from '@/shared/styles';

const aboutImage = require('../../../assets/images/nasreddin-schmieded-software.png');
const aboutMarkdown = require('../../../about.md');

type AboutScreenProps = {
  styles: AppStyles;
};

type MarkdownBlock =
  | { content: string; type: 'paragraph' }
  | { content: string; level: 1 | 2 | 3; type: 'heading' }
  | { content: string; type: 'listItem' };

type InlineMarkdownPart = {
  content: string;
  marker?: '**' | '*' | '`';
};

function parseMarkdown(markdown: string) {
  const blocks: MarkdownBlock[] = [];
  const pendingParagraph: string[] = [];

  const flushParagraph = () => {
    if (pendingParagraph.length === 0) {
      return;
    }

    blocks.push({ content: pendingParagraph.join(' '), type: 'paragraph' });
    pendingParagraph.length = 0;
  };

  markdown.split(/\r?\n/).forEach((line) => {
    const trimmedLine = line.trim();

    if (!trimmedLine) {
      flushParagraph();
      return;
    }

    const headingMatch = /^(#{1,3})\s+(.+)$/.exec(trimmedLine);

    if (headingMatch) {
      flushParagraph();
      blocks.push({
        content: headingMatch[2],
        level: headingMatch[1].length as 1 | 2 | 3,
        type: 'heading',
      });
      return;
    }

    const listItemMatch = /^[-*]\s+(.+)$/.exec(trimmedLine);

    if (listItemMatch) {
      flushParagraph();
      blocks.push({ content: listItemMatch[1], type: 'listItem' });
      return;
    }

    pendingParagraph.push(trimmedLine);
  });

  flushParagraph();

  return blocks;
}

function parseInlineMarkdown(text: string) {
  const parts: InlineMarkdownPart[] = [];
  const inlinePattern = /(\*\*[^*]+\*\*|`[^`]+`|\*[^*]+\*)/g;
  let lastIndex = 0;
  let match: RegExpExecArray | null;

  while ((match = inlinePattern.exec(text))) {
    if (match.index > lastIndex) {
      parts.push({ content: text.slice(lastIndex, match.index) });
    }

    const matchedText = match[0];

    if (matchedText.startsWith('**')) {
      parts.push({ content: matchedText.slice(2, -2), marker: '**' });
    } else if (matchedText.startsWith('`')) {
      parts.push({ content: matchedText.slice(1, -1), marker: '`' });
    } else {
      parts.push({ content: matchedText.slice(1, -1), marker: '*' });
    }

    lastIndex = match.index + matchedText.length;
  }

  if (lastIndex < text.length) {
    parts.push({ content: text.slice(lastIndex) });
  }

  return parts;
}

function MarkdownText({ styles, text }: { styles: AppStyles; text: string }) {
  return (
    <>
      {parseInlineMarkdown(text).map((part, index) => (
        <Text
          key={`${part.content}-${index}`}
          style={[
            part.marker === '**' && styles.markdownStrong,
            part.marker === '*' && styles.markdownEmphasis,
            part.marker === '`' && styles.markdownCode,
          ]}>
          {part.content}
        </Text>
      ))}
    </>
  );
}

export function AboutScreen({ styles }: AboutScreenProps) {
  const [markdownText, setMarkdownText] = useState('');
  const [loadingErrorText, setLoadingErrorText] = useState('');
  const markdownBlocks = useMemo(() => parseMarkdown(markdownText), [markdownText]);

  useEffect(() => {
    let shouldApplyMarkdown = true;

    async function loadMarkdown() {
      try {
        const markdownAsset = Asset.fromModule(aboutMarkdown);
        await markdownAsset.downloadAsync();
        const markdownResponse = await fetch(markdownAsset.localUri ?? markdownAsset.uri);
        const nextMarkdownText = await markdownResponse.text();

        if (shouldApplyMarkdown) {
          setMarkdownText(nextMarkdownText);
        }
      } catch {
        if (shouldApplyMarkdown) {
          setLoadingErrorText('Die About-Seite konnte nicht geladen werden.');
        }
      }
    }

    loadMarkdown();

    return () => {
      shouldApplyMarkdown = false;
    };
  }, []);

  return (
    <View style={styles.aboutPanel}>
      <Image resizeMode="contain" source={aboutImage} style={styles.aboutIcon} />
      <View style={styles.markdownContainer}>
        {loadingErrorText ? <Text style={styles.mutedText}>{loadingErrorText}</Text> : null}
        {!loadingErrorText && markdownBlocks.length === 0 ? (
          <Text style={styles.mutedText}>About-Seite wird geladen.</Text>
        ) : null}
        {markdownBlocks.map((block, index) => {
          if (block.type === 'heading') {
            return (
              <Text
                key={`${block.content}-${index}`}
                style={[
                  styles.markdownHeading,
                  block.level === 1 && styles.markdownHeadingOne,
                  block.level === 2 && styles.markdownHeadingTwo,
                  block.level === 3 && styles.markdownHeadingThree,
                ]}>
                <MarkdownText styles={styles} text={block.content} />
              </Text>
            );
          }

          if (block.type === 'listItem') {
            return (
              <View key={`${block.content}-${index}`} style={styles.markdownListRow}>
                <Text style={styles.markdownBullet}>•</Text>
                <Text style={styles.markdownListText}>
                  <MarkdownText styles={styles} text={block.content} />
                </Text>
              </View>
            );
          }

          return (
            <Text key={`${block.content}-${index}`} style={styles.markdownParagraph}>
              <MarkdownText styles={styles} text={block.content} />
            </Text>
          );
        })}
      </View>
    </View>
  );
}
