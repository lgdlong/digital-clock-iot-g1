"use client";
import Link from "next/link";
import { usePathname } from "next/navigation";

import "@/css/mobile-navbar.css";

const navs = [
  {
    href: "/",
    label: "Home",
    icon: <i className="bi bi-house" />,
  },
  {
    href: "/set-time",
    label: "Đặt giờ",
    icon: <i className="bi bi-clock" />,
  },
  {
    href: "/alarm",
    label: "Báo thức",
    icon: <i className="bi bi-alarm" />,
  },
  {
    href: "/settings",
    label: "Cài đặt",
    icon: <i className="bi bi-gear" />,
  },
];

export default function MobileNavbar() {
  const pathname = usePathname();

  return (
    <nav className="navbar fixed-bottom">
      <div className="container-fluid d-flex justify-content-around p-0">
        {navs.map((nav) => (
          <Link
            href={nav.href}
            key={nav.href}
            className={`flex-fill text-center py-2 px-1 text-white ${
              pathname === nav.href ? "fw-bold" : "opacity-75"
            }`}
            style={{
              fontSize: 18,
              textDecoration: "none",
              transition: "opacity 0.2s",
            }}
          >
            <div>{nav.icon}</div>
            <div className="nav-label">{nav.label}</div>
          </Link>
        ))}
      </div>
    </nav>
  );
}
