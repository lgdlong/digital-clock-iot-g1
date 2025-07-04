"use client";
import { useState, useEffect } from 'react';
import dynamic from 'next/dynamic';

// Dynamically import SwaggerUI to avoid SSR issues
const SwaggerUI = dynamic(() => import('swagger-ui-react'), { ssr: false });
import 'swagger-ui-react/swagger-ui.css';

export default function ApiDocsPage() {
  const [spec, setSpec] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    // Fetch the swagger spec from our API
    fetch('/api/swagger')
      .then(response => response.json())
      .then(data => {
        setSpec(data);
        setLoading(false);
      })
      .catch(error => {
        console.error('Error loading Swagger spec:', error);
        setLoading(false);
      });
  }, []);

  if (loading) {
    return (
      <div className="container py-5">
        <div className="text-center">
          <div className="spinner-border text-primary" role="status">
            <span className="visually-hidden">Loading...</span>
          </div>
          <p className="mt-3">Loading API Documentation...</p>
        </div>
      </div>
    );
  }

  if (!spec) {
    return (
      <div className="container py-5">
        <div className="alert alert-danger">
          <h4>Error</h4>
          <p>Failed to load API documentation. Please try again later.</p>
        </div>
      </div>
    );
  }

  return (
    <div className="container-fluid py-3">
      <div className="row">
        <div className="col-12">
          <div className="mb-3">
            <h1 className="text-primary">ESP32 Smart Clock API Documentation</h1>
            <p className="text-muted">
              Complete API documentation for the ESP32 Smart Clock system with alarm management and MQTT integration.
            </p>
          </div>
          <div className="bg-white rounded shadow-sm">
            <SwaggerUI spec={spec} />
          </div>
        </div>
      </div>
    </div>
  );
}
