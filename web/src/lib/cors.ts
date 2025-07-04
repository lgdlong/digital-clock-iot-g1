// ✅ Allow API access from all sources (Fix CORS 403 error)
import { NextResponse } from 'next/server';

export function addCorsHeaders(response: NextResponse) {
  // ✅ Allow access from any origin
  response.headers.set('Access-Control-Allow-Origin', '*');
  response.headers.set('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, DELETE');
  response.headers.set('Access-Control-Allow-Headers', 'Content-Type, Authorization');
  response.headers.set('Content-Type', 'application/json');
  
  return response;
}

// ✅ Handle OPTIONS requests so browser doesn't get blocked
export function handleOptionsRequest() {
  const response = new NextResponse(null, { status: 200 });
  return addCorsHeaders(response);
}

export function createCorsResponse(data?: any, status: number = 200) {
  const response = data 
    ? NextResponse.json(data, { status }) 
    : new NextResponse(null, { status });
    
  return addCorsHeaders(response);
}
