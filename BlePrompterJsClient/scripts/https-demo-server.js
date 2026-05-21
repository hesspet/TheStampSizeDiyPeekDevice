const fs = require("node:fs");
const https = require("node:https");
const os = require("node:os");
const path = require("node:path");

const port = Number.parseInt(process.argv[2] || "8443", 10);
const rootDirectory = path.resolve(__dirname, "..");
const certificateDirectory = path.join(rootDirectory, "certificates");
const serverCertificatePath = path.join(certificateDirectory, "ble-prompter-demo-server.pfx");
const passphrase = "BlePrompterDemo";

const mimeTypes = new Map([
  [".css", "text/css; charset=utf-8"],
  [".html", "text/html; charset=utf-8"],
  [".js", "text/javascript; charset=utf-8"],
  [".json", "application/json; charset=utf-8"],
  [".txt", "text/plain; charset=utf-8"],
]);

function getLocalIpv4Addresses() {
  return Object.values(os.networkInterfaces())
    .flat()
    .filter((address) => address && address.family === "IPv4" && !address.internal)
    .map((address) => address.address);
}

function getFilePath(requestUrl) {
  const parsedUrl = new URL(requestUrl, `https://localhost:${port}`);
  const decodedPath = decodeURIComponent(parsedUrl.pathname);
  const requestedPath = decodedPath === "/" ? "/index.html" : decodedPath;
  const filePath = path.normalize(path.join(rootDirectory, requestedPath));

  if (!filePath.startsWith(rootDirectory)) {
    return null;
  }

  return filePath;
}

const server = https.createServer(
  {
    pfx: fs.readFileSync(serverCertificatePath),
    passphrase,
  },
  (request, response) => {
    const filePath = getFilePath(request.url);

    if (!filePath) {
      response.writeHead(403, { "Content-Type": "text/plain; charset=utf-8" });
      response.end("Zugriff verweigert");
      return;
    }

    fs.readFile(filePath, (error, content) => {
      if (error) {
        response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
        response.end("Nicht gefunden");
        return;
      }

      response.writeHead(200, {
        "Content-Type": mimeTypes.get(path.extname(filePath)) || "application/octet-stream",
        "Cache-Control": "no-store",
      });
      response.end(content);
    });
  }
);

server.on("error", (error) => {
  if (error.code === "EADDRINUSE") {
    console.error(`Port ${port} ist bereits belegt.`);
    console.error("Beende den laufenden Server oder starte diese Batchdatei mit einem anderen Port.");
    process.exit(1);
  }

  console.error(`Serverstart fehlgeschlagen: ${error.message}`);
  process.exit(1);
});

server.listen(port, "0.0.0.0", () => {
  console.log("BlePrompter JS Client HTTPS-Demo-Server");
  console.log(`Lokal: https://localhost:${port}/`);
  for (const ipAddress of getLocalIpv4Addresses()) {
    console.log(`LAN:   https://${ipAddress}:${port}/`);
  }
  console.log("Beenden mit Strg+C");
});
