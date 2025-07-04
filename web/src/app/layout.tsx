import "bootstrap/dist/css/bootstrap.min.css";
import "bootstrap-icons/font/bootstrap-icons.css";
import "@/css/globals.css";
import type { Metadata } from "next";
import MobileNavbar from "@/components/MobileNavbar";

export const metadata: Metadata = {
  title: "ESP32 Alarm Web",
  description: "Điều khiển báo thức IoT với ESP32",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="vi" suppressHydrationWarning>
      <body className="text-white" suppressHydrationWarning>
        <header className="text-center p-3 fs-4 fw-bold">
          Tên thiết bị: <span className="text-primary">SmartClock001</span>
        </header>
        <main style={{ paddingBottom: 72 }}>{children}</main>
        <MobileNavbar />
      </body>
    </html>
  );
}
