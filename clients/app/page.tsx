'use client';

import { fetchDataFromCluster } from '@/components/Actions';
import { useState } from 'react';

type t={
    success: boolean;
    message: string;
    assignedPort?: number;
}

export default function Home() {
  const [loading, setLoading] = useState<boolean>(false);
  const [clusterResponse, setClusterResponse] = useState<t>();

  const handleRequest = async () => {
    setLoading(true);
    const result = await fetchDataFromCluster();
    setClusterResponse(result);
    setLoading(false);
  };

  return (
    <main style={{ padding: '3rem', fontFamily: 'sans-serif', maxWidth: '600px', margin: '0 auto' }}>
      <h1>Distributed System Dashboard</h1>
      <p style={{ color: '#666' }}>Next.js App Router ⇄ C++ Load Balancer (8080) ⇄ Node.js Cluster</p>
      
      <hr style={{ margin: '2rem 0', borderColor: '#eaeaea' }} />

      <button 
        onClick={handleRequest} 
        disabled={loading}
        style={{
          padding: '0.75rem 1.5rem',
          fontSize: '1rem',
          backgroundColor: '#0070f3',
          color: 'white',
          border: 'none',
          borderRadius: '5px',
          cursor: loading ? 'not-allowed' : 'pointer'
        }}
      >
        {loading ? 'Routing Request...' : 'Send Live Request'}
      </button>

      {clusterResponse && (
        <div style={{ marginTop: '2rem', padding: '1.5rem', border: '1px solid #ccc', borderRadius: '8px', backgroundColor: '#2f2727' }}>
          <h3>Cluster Execution Result:</h3>
          <p><strong>Status:</strong> {clusterResponse.success ? '🟢 Success' : '🔴 Failed'}</p>
          <p><strong>Server Message:</strong> "{clusterResponse.message}"</p>
          
          {clusterResponse.success && (
            <div style={{ marginTop: '1rem', display: 'inline-block', padding: '0.25rem 0.5rem', backgroundColor: '#e2f0d9', borderRadius: '4px', fontSize: '0.85rem' }}>
              Routed via Node Port: <strong>{clusterResponse.assignedPort}</strong>
            </div>
          )}
        </div>
      )}
    </main>
  );
}