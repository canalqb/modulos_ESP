import { defineConfig } from 'vite';
import { jsDelivr } from 'unplugin-jsdelivr';

export default defineConfig({
  plugins: [
    jsDelivr({
      modules: [
        // Bootstrap 5.3.3
        {
          module: 'bootstrap',
          version: '5.3.3',
          transform: (moduleName, importName) => {
            if (importName === 'default') {
              return `${moduleName}/dist/js/bootstrap.bundle.min.js`;
            }
            return `${moduleName}/dist/js/bootstrap.bundle.min.js`;
          }
        },
        // Font Awesome 6.5.0
        {
          module: '@fortawesome/fontawesome-free',
          version: '6.5.0',
          transform: (moduleName, importName) => {
            if (importName.includes('icon')) {
              return `${moduleName}/js/all.min.js`;
            }
            return `${moduleName}/js/all.min.js`;
          }
        }
      ],
      endpoint: 'npm'
    })
  ],
  build: {
    rollupOptions: {
      output: {
        manualChunks: {
          vendor: ['bootstrap'],
          icons: ['@fortawesome/fontawesome-free']
        }
      }
    },
    outDir: 'dist',
    assetsDir: 'assets'
  },
  server: {
    port: 3000,
    open: true
  }
});
