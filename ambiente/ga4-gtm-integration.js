/**
 * Google Analytics 4 + Google Tag Manager Integration
 * CanalQB ESP32 Dashboard - Advanced Tracking & Automation
 * 
 * Features:
 * - Enhanced event tracking
 * - Custom dimensions
 * - User behavior analysis
 * - Performance monitoring
 * - Conversion tracking
 * - Real-time insights
 */

(function() {
    'use strict';
    
    // Configuration
    const GTM_GA4_CONFIG = {
        containerId: 'GTM-W45V7S5Z',
        measurementId: 'G-XXXXXXXXXX', // GA4 Measurement ID
        propertyId: '392122309',
        serviceAccount: 'esp-624@bloggerwindsurf.iam.gserviceaccount.com',
        trackingEnabled: true,
        debugMode: false,
        customDimensions: {
            deviceType: 'custom_dimension_1',
            userRole: 'custom_dimension_2',
            featureUsage: 'custom_dimension_3',
            performanceScore: 'custom_dimension_4'
        }
    };
    
    // Enhanced Data Layer Manager
    window.CQBDataLayer = {
        events: [],
        userContext: {},
        sessionData: {},
        
        init() {
            console.log('Initializing CQB Data Layer Manager...');
            this.setupUserContext();
            this.trackPageView();
            this.setupEventListeners();
            this.startPerformanceTracking();
        },
        
        setupUserContext() {
            this.userContext = {
                userId: this.generateUserId(),
                sessionId: this.generateSessionId(),
                deviceType: this.detectDeviceType(),
                browser: this.detectBrowser(),
                esp32IP: window.S?.espIp || 'unknown',
                timestamp: new Date().toISOString(),
                firstVisit: !localStorage.getItem('cqb_returning_user')
            };
            
            if (this.userContext.firstVisit) {
                localStorage.setItem('cqb_returning_user', 'true');
                this.trackEvent('first_visit', {
                    device_type: this.userContext.deviceType,
                    browser: this.userContext.browser
                });
            }
            
            // Push to GTM data layer
            window.dataLayer = window.dataLayer || [];
            window.dataLayer.push({
                'event': 'user_context_ready',
                'user_context': this.userContext
            });
        },
        
        trackPageView(pageData = {}) {
            const pageDataEnhanced = {
                page_title: document.title,
                page_location: window.location.href,
                page_path: window.location.pathname,
                ...this.userContext,
                ...pageData
            };
            
            // GA4 page_view
            if (window.gtag) {
                gtag('config', GTM_GA4_CONFIG.measurementId, {
                    page_path: window.location.pathname,
                    custom_map: GTM_GA4_CONFIG.customDimensions
                });
                
                gtag('event', 'page_view', pageDataEnhanced);
            }
            
            // Custom tracking
            this.trackEvent('page_view', pageDataEnhanced);
        },
        
        trackEvent(eventName, parameters = {}) {
            const eventData = {
                event_name: eventName,
                ...parameters,
                ...this.userContext,
                timestamp: new Date().toISOString()
            };
            
            // Push to GTM
            if (window.dataLayer) {
                window.dataLayer.push({
                    'event': eventName,
                    'event_data': eventData
                });
            }
            
            // GA4 custom event
            if (window.gtag) {
                gtag('event', eventName, parameters);
            }
            
            console.log('Event tracked:', eventName, eventData);
        },
        
        setupEventListeners() {
            // Menu interactions
            document.addEventListener('click', (e) => {
                const navLink = e.target.closest('.cqb-nav-link, .cqb-nav-sub');
                if (navLink) {
                    const linkText = navLink.querySelector('.nav-lbl')?.textContent || 'Unknown';
                    this.trackEvent('menu_click', {
                        menu_item: linkText,
                        menu_type: navLink.classList.contains('cqb-nav-sub') ? 'submenu' : 'main'
                    });
                }
            });
            
            // Form submissions
            document.addEventListener('submit', (e) => {
                const form = e.target;
                const formName = form.getAttribute('data-name') || 'unknown_form';
                this.trackEvent('form_submit', {
                    form_name: formName,
                    form_id: form.id || 'no_id'
                });
            });
            
            // Button interactions
            document.addEventListener('click', (e) => {
                const button = e.target.closest('.cqb-btn');
                if (button) {
                    this.trackEvent('button_click', {
                        button_text: button.textContent.trim(),
                        button_type: this.getButtonType(button),
                        button_id: button.id || 'no_id'
                    });
                }
            });
            
            // Tab changes
            const observer = new MutationObserver((mutations) => {
                mutations.forEach((mutation) => {
                    if (mutation.target.classList && mutation.target.classList.contains('cqb-tab-active')) {
                        this.trackEvent('tab_change', {
                            tab_name: mutation.target.id || 'unknown_tab'
                        });
                    }
                });
            });
            
            document.querySelectorAll('.cqb-tab').forEach(tab => {
                observer.observe(tab, { attributes: true, attributeFilter: ['class'] });
            });
        },
        
        startPerformanceTracking() {
            // Track page load performance
            window.addEventListener('load', () => {
                setTimeout(() => {
                    const perfData = this.getPerformanceData();
                    this.trackEvent('page_performance', perfData);
                    
                    // Performance score for optimization
                    const performanceScore = this.calculatePerformanceScore(perfData);
                    this.trackEvent('performance_score', {
                        score: performanceScore,
                        grade: this.getPerformanceGrade(performanceScore)
                    });
                }, 1000);
            });
            
            // Track ESP32 response times
            this.trackESP32Performance();
        },
        
        trackESP32Performance() {
            const originalFetch = window.fetch;
            window.fetch = async function(...args) {
                const startTime = performance.now();
                const url = args[0];
                
                if (url.includes(window.S?.espIp || '192.168')) {
                    try {
                        const response = await originalFetch.apply(this, args);
                        const endTime = performance.now();
                        const duration = endTime - startTime;
                        
                        window.CQBDataLayer.trackEvent('esp32_api_call', {
                            endpoint: url,
                            response_time: Math.round(duration),
                            status: response.status,
                            success: response.ok
                        });
                        
                        return response;
                    } catch (error) {
                        const endTime = performance.now();
                        const duration = endTime - startTime;
                        
                        window.CQBDataLayer.trackEvent('esp32_api_error', {
                            endpoint: url,
                            response_time: Math.round(duration),
                            error: error.message
                        });
                        
                        throw error;
                    }
                }
                
                return originalFetch.apply(this, args);
            };
        },
        
        // Utility methods
        generateUserId() {
            let userId = localStorage.getItem('cqb_user_id');
            if (!userId) {
                userId = 'user_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
                localStorage.setItem('cqb_user_id', userId);
            }
            return userId;
        },
        
        generateSessionId() {
            let sessionId = sessionStorage.getItem('cqb_session_id');
            if (!sessionId) {
                sessionId = 'session_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
                sessionStorage.setItem('cqb_session_id', sessionId);
            }
            return sessionId;
        },
        
        detectDeviceType() {
            const userAgent = navigator.userAgent;
            if (/Mobi|Android/i.test(userAgent)) return 'mobile';
            if (/Tablet|iPad/i.test(userAgent)) return 'tablet';
            return 'desktop';
        },
        
        detectBrowser() {
            const userAgent = navigator.userAgent;
            if (userAgent.includes('Chrome')) return 'chrome';
            if (userAgent.includes('Firefox')) return 'firefox';
            if (userAgent.includes('Safari')) return 'safari';
            if (userAgent.includes('Edge')) return 'edge';
            return 'unknown';
        },
        
        getButtonType(button) {
            if (button.classList.contains('cqb-btn-primary')) return 'primary';
            if (button.classList.contains('cqb-btn-secondary')) return 'secondary';
            if (button.classList.contains('cqb-btn-danger')) return 'danger';
            if (button.classList.contains('cqb-btn-success')) return 'success';
            return 'default';
        },
        
        getPerformanceData() {
            const navigation = performance.getEntriesByType('navigation')[0];
            return {
                load_time: Math.round(navigation.loadEventEnd - navigation.fetchStart),
                dom_content_loaded: Math.round(navigation.domContentLoadedEventEnd - navigation.fetchStart),
                first_paint: this.getMetric('first-paint'),
                first_contentful_paint: this.getMetric('first-contentful-paint'),
                largest_contentful_paint: this.getMetric('largest-contentful-paint'),
                cumulative_layout_shift: this.getMetric('cumulative-layout-shift')
            };
        },
        
        getMetric(name) {
            const entries = performance.getEntriesByType('paint');
            const entry = entries.find(e => e.name === name);
            return entry ? Math.round(entry.startTime) : 0;
        },
        
        calculatePerformanceScore(perfData) {
            let score = 100;
            
            // Load time penalty
            if (perfData.load_time > 3000) score -= 20;
            else if (perfData.load_time > 2000) score -= 10;
            
            // FCP penalty
            if (perfData.first_contentful_paint > 2000) score -= 15;
            else if (perfData.first_contentful_paint > 1000) score -= 5;
            
            // LCP penalty
            if (perfData.largest_contentful_paint > 4000) score -= 20;
            else if (perfData.largest_contentful_paint > 2500) score -= 10;
            
            return Math.max(0, score);
        },
        
        getPerformanceGrade(score) {
            if (score >= 90) return 'A';
            if (score >= 80) return 'B';
            if (score >= 70) return 'C';
            if (score >= 60) return 'D';
            return 'F';
        }
    };
    
    // Advanced ESP32 Feature Tracking
    window.CQBFeatureTracking = {
        features: {},
        
        init() {
            this.trackWiFiConnections();
            this.trackConfigurationChanges();
            this.trackSystemStatus();
            this.trackUserPreferences();
        },
        
        trackWiFiConnections() {
            // Override connectWiFi function to track connections
            if (window.connectWiFi) {
                const originalConnect = window.connectWiFi;
                window.connectWiFi = async function(ssid, password) {
                    window.CQBFeatureTracking.trackFeature('wifi_connection_attempt', {
                        ssid: ssid,
                        timestamp: new Date().toISOString()
                    });
                    
                    try {
                        const result = await originalConnect.call(this, ssid, password);
                        window.CQBFeatureTracking.trackFeature('wifi_connection_success', {
                            ssid: ssid,
                            timestamp: new Date().toISOString()
                        });
                        return result;
                    } catch (error) {
                        window.CQBFeatureTracking.trackFeature('wifi_connection_failed', {
                            ssid: ssid,
                            error: error.message,
                            timestamp: new Date().toISOString()
                        });
                        throw error;
                    }
                };
            }
        },
        
        trackConfigurationChanges() {
            // Track all configuration changes
            const configFunctions = ['saveWifiConfig', 'saveSsidConfig', 'saveGoogleConfig'];
            
            configFunctions.forEach(funcName => {
                if (window[funcName]) {
                    const originalFunc = window[funcName];
                    window[funcName] = function(...args) {
                        window.CQBFeatureTracking.trackFeature('config_change', {
                            function: funcName,
                            timestamp: new Date().toISOString(),
                            args_count: args.length
                        });
                        
                        return originalFunc.apply(this, args);
                    };
                }
            });
        },
        
        trackSystemStatus() {
            // Track system status checks
            setInterval(() => {
                if (window.S && window.S.espIp) {
                    fetch(`http://${window.S.espIp}/api/status`)
                        .then(response => response.json())
                        .then(status => {
                            window.CQBFeatureTracking.trackFeature('system_status', {
                                uptime: status.uptime,
                                free_heap: status.free_heap,
                                wifi_strength: status.rssi,
                                timestamp: new Date().toISOString()
                            });
                        })
                        .catch(error => {
                            console.log('Status check failed:', error);
                        });
                }
            }, 60000); // Every minute
        },
        
        trackUserPreferences() {
            // Track theme changes
            const originalApplyTheme = window.applyTheme;
            window.applyTheme = function(theme) {
                window.CQBFeatureTracking.trackFeature('theme_change', {
                    theme: theme,
                    timestamp: new Date().toISOString()
                });
                
                return originalApplyTheme.call(this, theme);
            };
            
            // Track menu interactions
            document.addEventListener('click', (e) => {
                const menuLink = e.target.closest('[onclick*="showSection"]');
                if (menuLink) {
                    const section = menuLink.getAttribute('onclick')?.match(/showSection\('([^']+)'\)/);
                    if (section) {
                        window.CQBFeatureTracking.trackFeature('section_access', {
                            section: section[1],
                            timestamp: new Date().toISOString()
                        });
                    }
                }
            });
        },
        
        trackFeature(featureName, data) {
            this.features[featureName] = this.features[featureName] || [];
            this.features[featureName].push(data);
            
            window.CQBDataLayer.trackEvent('feature_usage', {
                feature_name: featureName,
                feature_data: data
            });
        }
    };
    
    // Real-time Analytics Dashboard
    window.CQBAnalyticsDashboard = {
        isVisible: false,
        metrics: {},
        
        init() {
            this.createDashboard();
            this.setupKeyboardShortcut();
            this.startRealTimeUpdates();
        },
        
        createDashboard() {
            const dashboard = document.createElement('div');
            dashboard.id = 'cqb-analytics-dashboard';
            dashboard.style.cssText = `
                position: fixed;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
                width: 600px;
                max-width: 90vw;
                height: 400px;
                background: var(--bg-primary);
                border: 2px solid var(--border-color);
                border-radius: 12px;
                padding: 20px;
                z-index: 10000;
                display: none;
                box-shadow: 0 10px 40px rgba(0,0,0,0.3);
                overflow-y: auto;
            `;
            
            dashboard.innerHTML = `
                <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px;">
                    <h3 style="margin: 0; color: var(--text-primary);">
                        <i class="fas fa-chart-line"></i> Analytics Dashboard
                    </h3>
                    <button onclick="window.CQBAnalyticsDashboard.toggle()" class="cqb-btn cqb-btn-ghost">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                
                <div id="analytics-content" style="display: grid; gap: 15px;">
                    <div class="cqb-card">
                        <div class="cqb-card-header">
                            <i class="fas fa-users hd-icon"></i> User Activity
                        </div>
                        <div class="cqb-card-body">
                            <div id="user-metrics">Loading...</div>
                        </div>
                    </div>
                    
                    <div class="cqb-card">
                        <div class="cqb-card-header">
                            <i class="fas fa-tachometer-alt hd-icon"></i> Performance
                        </div>
                        <div class="cqb-card-body">
                            <div id="performance-metrics">Loading...</div>
                        </div>
                    </div>
                    
                    <div class="cqb-card">
                        <div class="cqb-card-header">
                            <i class="fas fa-cog hd-icon"></i> Feature Usage
                        </div>
                        <div class="cqb-card-body">
                            <div id="feature-metrics">Loading...</div>
                        </div>
                    </div>
                </div>
            `;
            
            document.body.appendChild(dashboard);
        },
        
        setupKeyboardShortcut() {
            document.addEventListener('keydown', (e) => {
                if (e.ctrlKey && e.shiftKey && e.key === 'A') {
                    this.toggle();
                }
            });
        },
        
        toggle() {
            this.isVisible = !this.isVisible;
            const dashboard = document.getElementById('cqb-analytics-dashboard');
            dashboard.style.display = this.isVisible ? 'block' : 'none';
            
            if (this.isVisible) {
                this.updateMetrics();
            }
        },
        
        startRealTimeUpdates() {
            setInterval(() => {
                if (this.isVisible) {
                    this.updateMetrics();
                }
            }, 5000); // Update every 5 seconds
        },
        
        updateMetrics() {
            this.updateUserMetrics();
            this.updatePerformanceMetrics();
            this.updateFeatureMetrics();
        },
        
        updateUserMetrics() {
            const userMetrics = document.getElementById('user-metrics');
            if (userMetrics && window.CQBDataLayer) {
                const context = window.CQBDataLayer.userContext;
                userMetrics.innerHTML = `
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px; font-size: 0.9rem;">
                        <div><strong>User ID:</strong> ${context.userId}</div>
                        <div><strong>Session:</strong> ${context.sessionId}</div>
                        <div><strong>Device:</strong> ${context.deviceType}</div>
                        <div><strong>Browser:</strong> ${context.browser}</div>
                        <div><strong>ESP32 IP:</strong> ${context.esp32IP}</div>
                        <div><strong>First Visit:</strong> ${context.firstVisit ? 'Yes' : 'No'}</div>
                    </div>
                `;
            }
        },
        
        updatePerformanceMetrics() {
            const perfMetrics = document.getElementById('performance-metrics');
            if (perfMetrics) {
                const perfData = window.CQBDataLayer.getPerformanceData();
                const score = window.CQBDataLayer.calculatePerformanceScore(perfData);
                perfMetrics.innerHTML = `
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 10px; font-size: 0.9rem;">
                        <div><strong>Load Time:</strong> ${perfData.load_time}ms</div>
                        <div><strong>Performance Score:</strong> ${score}/100</div>
                        <div><strong>FCP:</strong> ${perfData.first_contentful_paint}ms</div>
                        <div><strong>Grade:</strong> ${window.CQBDataLayer.getPerformanceGrade(score)}</div>
                    </div>
                `;
            }
        },
        
        updateFeatureMetrics() {
            const featureMetrics = document.getElementById('feature-metrics');
            if (featureMetrics && window.CQBFeatureTracking) {
                const features = window.CQBFeatureTracking.features;
                const featureList = Object.keys(features).map(key => {
                    const count = features[key].length;
                    const lastUsed = features[key][features[key].length - 1]?.timestamp || 'Never';
                    return `<div><strong>${key}:</strong> ${count} uses (last: ${new Date(lastUsed).toLocaleTimeString()})</div>`;
                }).join('');
                
                featureMetrics.innerHTML = `<div style="font-size: 0.9rem;">${featureList || 'No features tracked yet'}</div>`;
            }
        }
    };
    
    // Initialize everything when DOM is ready
    document.addEventListener('DOMContentLoaded', () => {
        // Wait a bit for other scripts to load
        setTimeout(() => {
            console.log('Initializing CQB Analytics Integration...');
            
            // Initialize data layer
            window.CQBDataLayer.init();
            
            // Initialize feature tracking
            window.CQBFeatureTracking.init();
            
            // Initialize analytics dashboard
            window.CQBAnalyticsDashboard.init();
            
            console.log('CQB Analytics Integration ready! Press Ctrl+Shift+A to open dashboard.');
        }, 1000);
    });
    
    // Export for external use
    window.CQBAnalytics = {
        DataLayer: window.CQBDataLayer,
        FeatureTracking: window.CQBFeatureTracking,
        Dashboard: window.CQBAnalyticsDashboard
    };
    
})();
