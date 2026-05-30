import type { GestureResponderEvent } from 'react-native';

export function getPinchDistance(event: GestureResponderEvent) {
  const [firstTouch, secondTouch] = event.nativeEvent.touches;

  if (!firstTouch || !secondTouch) {
    return null;
  }

  const horizontalDistance = firstTouch.pageX - secondTouch.pageX;
  const verticalDistance = firstTouch.pageY - secondTouch.pageY;

  return Math.sqrt(horizontalDistance * horizontalDistance + verticalDistance * verticalDistance);
}
