import datetime as datetime_module
import json
import socket
import tempfile
from pathlib import Path


LISTEN_ADDRESS = "0.0.0.0"
LISTEN_PORT = 4210
BUFFER_SIZE = 4096
PROGRAM_NAME = "EnduranceTestClient"


def create_protocol_file_path() -> Path:
    timestamp = datetime_module.datetime.now().strftime("%Y%m%d_%H%M%S")
    return Path(tempfile.gettempdir()) / f"endurance_test_{timestamp}.jsonl"


def get_received_timestamp() -> str:
    return datetime_module.datetime.now(datetime_module.timezone.utc).astimezone().isoformat(timespec="milliseconds")


def decode_payload(payload: bytes) -> str:
    return payload.decode("utf-8", errors="replace").strip()


def build_protocol_record(payload_text: str, sender_address: tuple[str, int]) -> dict:
    protocol_record = {
        "received_at": get_received_timestamp(),
        "sender_ip": sender_address[0],
        "sender_port": sender_address[1],
        "raw": payload_text,
    }

    try:
        protocol_record["data"] = json.loads(payload_text)
    except json.JSONDecodeError as error:
        protocol_record["parse_error"] = str(error)

    return protocol_record


def append_protocol_record(protocol_file_path: Path, protocol_record: dict) -> None:
    with protocol_file_path.open("a", encoding="utf-8") as protocol_file:
        json.dump(protocol_record, protocol_file, ensure_ascii=False, separators=(",", ":"))
        protocol_file.write("\n")


def print_start_message(protocol_file_path: Path) -> None:
    print(f"{PROGRAM_NAME} lauscht auf UDP {LISTEN_ADDRESS}:{LISTEN_PORT}")
    print(f"Protokolldatei: {protocol_file_path}")
    print("Abbruch mit Strg+C")
    print()


def run_receiver() -> None:
    protocol_file_path = create_protocol_file_path()
    print_start_message(protocol_file_path)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        udp_socket.bind((LISTEN_ADDRESS, LISTEN_PORT))

        while True:
            payload, sender_address = udp_socket.recvfrom(BUFFER_SIZE)
            payload_text = decode_payload(payload)
            protocol_record = build_protocol_record(payload_text, sender_address)
            append_protocol_record(protocol_file_path, protocol_record)

            print(f"[{protocol_record['received_at']}] {sender_address[0]}:{sender_address[1]}")
            print(payload_text)
            print(f"Geschrieben nach: {protocol_file_path}")
            print()


if __name__ == "__main__":
    try:
        run_receiver()
    except KeyboardInterrupt:
        print()
        print("Empfang beendet.")
