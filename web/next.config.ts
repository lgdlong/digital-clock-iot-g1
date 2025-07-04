import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  /* config options here */
  devIndicators: false, // ẩn Dev Tools UI của Next.js
  
  // Tắt ESLint trong build để tránh lỗi
  eslint: {
    ignoreDuringBuilds: true,
  },
};

export default nextConfig;
