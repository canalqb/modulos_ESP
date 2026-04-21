/**
 * Secure Configuration Loader for CanalQB ESP32
 * 
 * SECURITY NOTICE:
 * This file should be loaded from a secure backend or environment variables
 * Never commit sensitive data to public repositories
 * 
 * Usage:
 * 1. Deploy this to a secure server
 * 2. Load via: <script src="https://your-secure-server.com/secure-config.js"></script>
 * 3. Or inject via server-side template
 */

(function() {
    'use strict';
    
    // Production configuration - should be loaded from secure backend
    const PROD_CONFIG = {
        // Google Analytics 4
        GA4_MEASUREMENT_ID: 'G-XXXXXXXXXX', // Replace with actual ID
        GA4_PROPERTY_ID: '392122309',
        
        // Google Tag Manager
        GTM_CONTAINER_ID: 'GTM-W45V7S5Z',
        
        // Service Account (NEVER expose in frontend)
        SERVICE_ACCOUNT: null, // Load from backend only
        
        // API Endpoints
        ANALYTICS_ENDPOINT: 'https://canalqb-analytics.onrender.com/api/analytics',
        
        // Security
        DEBUG_MODE: false,
        TRACKING_ENABLED: true
    };
    
    // Development configuration
    const DEV_CONFIG = {
        ...PROD_CONFIG,
        DEBUG_MODE: true,
        GA4_MEASUREMENT_ID: 'G-XXXXXXXXXX_DEV' // Development ID
    };
    
    // Environment detection
    const isDevelopment = window.location.hostname === 'localhost' || 
                        window.location.hostname === '127.0.0.1' ||
                        window.location.hostname.includes('dev');
    
    // Select appropriate configuration
    const CONFIG = isDevelopment ? DEV_CONFIG : PROD_CONFIG;
    
    // Expose configuration globally
    window.CQB_CONFIG = CONFIG;
    
    // Backward compatibility
    window.CQB_GA4_MEASUREMENT_ID = CONFIG.GA4_MEASUREMENT_ID;
    window.CQB_GA4_PROPERTY_ID = CONFIG.GA4_PROPERTY_ID;
    window.CQB_SERVICE_ACCOUNT = CONFIG.SERVICE_ACCOUNT;
    
    // Security validation
    function validateConfig() {
        const required = ['GA4_MEASUREMENT_ID', 'GTM_CONTAINER_ID'];
        const missing = required.filter(key => !CONFIG[key] || CONFIG[key].includes('XXXX'));
        
        if (missing.length > 0) {
            console.warn('CQB Security: Missing configuration for:', missing);
            console.warn('Please configure secure variables in production environment');
        }
    }
    
    // Initialize security validation
    validateConfig();
    
    // Log initialization (only in development)
    if (CONFIG.DEBUG_MODE) {
        console.log('CQB Secure Configuration loaded:', {
            environment: isDevelopment ? 'development' : 'production',
            ga4Configured: !CONFIG.GA4_MEASUREMENT_ID.includes('XXXX'),
            gtmConfigured: !CONFIG.GTM_CONTAINER_ID.includes('XXXX'),
            serviceAccountSecure: CONFIG.SERVICE_ACCOUNT === null
        });
    }
    
})();
