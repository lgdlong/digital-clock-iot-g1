// ✅ Next.js Middleware for CORS (equivalent to PHP CORS headers)
import { NextRequest, NextResponse } from 'next/server';

export function middleware(request: NextRequest) {
  // ✅ Handle OPTIONS requests (preflight)
  if (request.method === 'OPTIONS') {
    const response = new NextResponse(null, { status: 200 });
    
    // ✅ Allow access from any origin (Fix CORS 403 error)
    response.headers.set('Access-Control-Allow-Origin', '*');
    response.headers.set('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, DELETE');
    response.headers.set('Access-Control-Allow-Headers', 'Content-Type, Authorization');
    
    return response;
  }

  // ✅ Add CORS headers to all API responses
  const response = NextResponse.next();
  
  response.headers.set('Access-Control-Allow-Origin', '*');
  response.headers.set('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, DELETE');
  response.headers.set('Access-Control-Allow-Headers', 'Content-Type, Authorization');
  
  return response;
}

// ✅ Apply middleware only to API routes
export const config = {
  matcher: '/api/:path*',
};
