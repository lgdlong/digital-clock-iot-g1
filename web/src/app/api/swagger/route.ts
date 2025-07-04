import { NextResponse } from 'next/server';
import specs from '@/lib/swagger';
import { createCorsResponse, handleOptionsRequest } from '@/lib/cors';

export async function GET() {
  return createCorsResponse(specs);
}

export async function OPTIONS() {
  return handleOptionsRequest();
}
