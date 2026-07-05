'use server';

import GetBestBackend from "./GetBestBackend";

type back={
    ip:string;
    port:number;
};

export async function fetchDataFromCluster() {
  try {
    const backend:back = await GetBestBackend();
    console.log(`Balancer redirected us to port: ${backend.port}`);

    // 2. Query that specific Node.js server directly for the page data
    const res = await fetch(`http://${backend.ip}:${backend.port}/`, {
      cache: 'no-store' // Don't cache so we can see the load shifting in real-time
    });

    if (!res.ok) throw new Error('Backend failed to respond');
    
    const textData = await res.text();
    return {
      success: true,
      message: textData,
      assignedPort: backend.port
    };

  } catch (error) {
    console.error("Routing Error:", error);
    return {
      success: false,
      message: "Could not establish cluster connection. Is the C++ infrastructure online?"
    };
  }
}