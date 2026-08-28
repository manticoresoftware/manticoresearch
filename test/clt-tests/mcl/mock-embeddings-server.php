#!/usr/bin/env php
<?php
/**
 * Mock embeddings API server for testing custom API_URL functionality.
 *
 * This server mimics OpenAI, Voyage, and Jina API endpoints and returns
 * deterministic random embeddings seeded by input text. It can be used to test the
 * API_URL parameter without making real API calls.
 *
 * Usage:
 *     php mock-embeddings-server.php --port 8080 --provider openai
 *     php mock-embeddings-server.php --port 8081 --provider voyage --delay 2.0
 *     php mock-embeddings-server.php --port 8082 --provider jina --delay 1.5
 *     php mock-embeddings-server.php --help
 *
 * The server accepts POST requests to /v1/embeddings and returns:
 * {
 *     "data": [
 *         {
 *             "embedding": [0.123, -0.456, ...]  // Deterministic random values seeded by input text
 *         }
 *     ]
 * }
 *
 * Features:
 * - Generates deterministic random vectors seeded by input text (same text = same vector)
 * - Supports configurable delay via command line (--delay) or input text ("delay=N")
 * - Supports provider setting via command line (--provider) or input text ("provider=...")
 * - Returns embeddings with correct dimensions based on model name
 * - Validates API keys: all keys are valid except "wrong" (returns 401 error)
 * - Validates model/provider matching: when provider is specified, only models for that provider are accepted
 *   - OpenAI provider: accepts models starting with "text-embedding"
 *   - Voyage provider: accepts models starting with "voyage-"
 *   - Jina provider: accepts models starting with "jina-" or "jina/"
 *   - If provider is not specified, any model is accepted (for generic testing)
 */

// Expected dimensions for each model
$MODEL_DIMENSIONS = [
    // OpenAI models
    'text-embedding-ada-002' => 1536,
    'text-embedding-3-small' => 1536,
    'text-embedding-3-large' => 3072,
    // Voyage models
    'voyage-3-large' => 1024,
    'voyage-3.5' => 1024,
    'voyage-3.5-lite' => 1024,
    'voyage-code-3' => 1024,
    'voyage-finance-2' => 1024,
    'voyage-law-2' => 1024,
    'voyage-code-2' => 1536,
    // Jina models
    'jina-embeddings-v4' => 2048,
    'jina-clip-v2' => 1024,
    'jina-embeddings-v3' => 1024,
    'jina-colbert-v2' => 128,
    'jina-clip-v1' => 768,
    'jina-colbert-v1-en' => 128,
    'jina-embeddings-v2-base-es' => 768,
    'jina-embeddings-v2-base-code' => 768,
    'jina-embeddings-v2-base-de' => 768,
    'jina-embeddings-v2-base-zh' => 768,
    'jina-embeddings-v2-base-en' => 768,
];

$DEFAULT_DIMENSIONS = 1536; // Default if model not found

/**
 * Generate a deterministic random embedding vector seeded by the input text.
 *
 * @param string $text Input text string
 * @param int $dimensions Number of dimensions for the embedding
 * @param int $seedOffset Optional offset to vary embeddings for different models
 * @return array List of floats representing the embedding vector
 */
function generate_embedding($text, $dimensions, $seedOffset = 0) {
    // Create a hash from the text to use as a seed
    $textHash = hash('sha256', $text);
    // Convert first 16 hex characters to integer for seeding
    $seed = hexdec(substr($textHash, 0, 16)) + $seedOffset;
    
    // Use a simple linear congruential generator for deterministic randomness
    // This ensures same text always produces same embedding
    $a = 1664525;
    $c = 1013904223;
    $m = 2**32;
    
    $embedding = [];
    $current = $seed;
    for ($i = 0; $i < $dimensions; $i++) {
        $current = ($a * $current + $c) % $m;
        // Normalize to range [-1.0, 1.0] and scale to typical embedding range
        $value = ($current / $m) * 2.0 - 1.0;
        // Scale to typical embedding magnitude (most embeddings are in [-1, 1] range)
        $value = $value * 0.5;
        $embedding[] = round($value, 6); // Round to 6 decimal places
    }
    
    return $embedding;
}

/**
 * Extract delay parameter from input text.
 * Looks for "delay=N" substring where N is a number.
 *
 * @param string $text Input text
 * @return float|null Delay in seconds, or null if not found
 */
function extract_delay_from_text($text) {
    if (preg_match('/delay=(\d+(?:\.\d+)?)/i', $text, $matches)) {
        return (float)$matches[1];
    }
    return null;
}

/**
 * Extract provider parameter from input text.
 * Looks for "provider=..." substring.
 *
 * @param string $text Input text
 * @return string|null Provider name, or null if not found
 */
function extract_provider_from_text($text) {
    if (preg_match('/provider=(\w+)/i', $text, $matches)) {
        return strtolower($matches[1]);
    }
    return null;
}

/**
 * Extract response type parameter from input text.
 * Looks for "response-type=..." substring.
 * Valid values: "html", "invalid-json", "wrong-structure", "default" (or null for default)
 *
 * @param string $text Input text
 * @return string|null Response type, or null if not found
 */
function extract_response_type_from_text($text) {
    if (preg_match('/response-type=(\w+(?:-\w+)?)/i', $text, $matches)) {
        return strtolower($matches[1]);
    }
    return null;
}

/**
 * Remove delay, provider, and response-type parameters from input text for embedding generation.
 *
 * @param string $text Input text
 * @return string Cleaned text without delay/provider/response-type parameters
 */
function clean_input_text($text) {
    // Remove delay=N, provider=..., and response-type=... substrings
    $text = preg_replace('/delay=\d+(?:\.\d+)?/i', '', $text);
    $text = preg_replace('/provider=\w+/i', '', $text);
    $text = preg_replace('/response-type=\w+(?:-\w+)?/i', '', $text);
    // Clean up any extra whitespace
    $text = trim($text);
    return $text;
}

/**
 * Validate that model matches provider.
 *
 * @param string $model Model name
 * @param string|null $provider Provider name (openai, voyage, jina, or null)
 * @return bool True if model matches provider or provider is not specified
 */
function validate_model_provider($model, $provider) {
    if ($provider === null) {
        return true; // No provider specified, accept any model
    }
    
    switch ($provider) {
        case 'openai':
            return strpos($model, 'text-embedding') === 0;
        case 'voyage':
            return strpos($model, 'voyage-') === 0;
        case 'jina':
            return strpos($model, 'jina-') === 0 || strpos($model, 'jina/') === 0;
        default:
            return true; // Unknown provider, accept any model
    }
}


/**
 * Handle POST request to /v1/embeddings.
 *
 * @param array $headers HTTP headers
 * @param string $body Request body
 * @param float $defaultDelay Default delay from command line
 * @param string|null $defaultProvider Default provider from command line
 * @return array Response array with 'code', 'headers', and 'body'
 */
function handle_embeddings_request($headers, $body, $defaultDelay, $defaultProvider) {
    // Validate API key from Authorization header
    $authHeader = $headers['Authorization'] ?? $headers['authorization'] ?? '';
    if (!preg_match('/^Bearer (.+)$/', $authHeader, $matches)) {
        return [
            'code' => 401,
            'headers' => ['Content-Type: application/json'],
            'body' => json_encode([
                'error' => [
                    'message' => 'Unauthorized: Missing or invalid Authorization header',
                    'type' => 'invalid_request_error',
                    'code' => 'invalid_api_key'
                ]
            ])
        ];
    }
    
    $apiKey = $matches[1];
    if ($apiKey === 'wrong') {
        // Return API error response for invalid key
        return [
            'code' => 401,
            'headers' => ['Content-Type: application/json'],
            'body' => json_encode([
                'error' => [
                    'message' => 'Invalid API key',
                    'type' => 'invalid_request_error',
                    'code' => 'invalid_api_key'
                ]
            ])
        ];
    }
    
    // Parse request body
    $requestData = json_decode($body, true);
    
    if ($requestData === null) {
        return [
            'code' => 400,
            'headers' => ['Content-Type: application/json'],
            'body' => json_encode([
                'error' => [
                    'message' => 'Invalid JSON',
                    'type' => 'invalid_request_error',
                    'code' => 'invalid_json'
                ]
            ])
        ];
    }
    
    // Extract model name and input
    $model = $requestData['model'] ?? '';
    $inputData = $requestData['input'] ?? [];
    
    // Handle both single string and array of strings
    if (is_string($inputData)) {
        $inputData = [$inputData];
    }
    
    // Extract delay, provider, and response-type from input text (if present)
    // These override command line defaults
    $delay = $defaultDelay;
    $provider = $defaultProvider;
    $responseType = null; // null means default (valid JSON)
    
    foreach ($inputData as $text) {
        $textDelay = extract_delay_from_text($text);
        if ($textDelay !== null) {
            $delay = $textDelay;
        }
        
        $textProvider = extract_provider_from_text($text);
        if ($textProvider !== null) {
            $provider = $textProvider;
        }
        
        $textResponseType = extract_response_type_from_text($text);
        if ($textResponseType !== null) {
            $responseType = $textResponseType;
        }
    }
    
    // Validate model matches provider (using extracted or default provider)
    if (!validate_model_provider($model, $provider)) {
        $providerName = ucfirst($provider);
        return [
            'code' => 400,
            'headers' => ['Content-Type: application/json'],
            'body' => json_encode([
                'error' => [
                    'message' => "Model '{$model}' is not a valid {$providerName} model",
                    'type' => 'invalid_request_error',
                    'code' => 'invalid_model'
                ]
            ])
        ];
    }
    
    // Determine dimensions based on model
    global $MODEL_DIMENSIONS, $DEFAULT_DIMENSIONS;
    $dimensions = $MODEL_DIMENSIONS[$model] ?? $DEFAULT_DIMENSIONS;
    
    // Apply delay if configured (simulates slow API responses)
    if ($delay > 0) {
        usleep((int)($delay * 1000000)); // Convert seconds to microseconds
    }
    
    // Handle response type override (if specified in input text)
    if ($responseType !== null) {
        switch ($responseType) {
            case 'html':
                // Return HTML instead of JSON
                return [
                    'code' => 200,
                    'headers' => ['Content-Type: text/html'],
                    'body' => '<html><body><h1>Not a JSON response</h1></body></html>'
                ];
            
            case 'invalid-json':
                // Return invalid JSON (malformed)
                return [
                    'code' => 200,
                    'headers' => ['Content-Type: application/json'],
                    'body' => '{"data": [{"embedding": [0.1, 0.2' // Incomplete JSON, missing closing brackets
                ];
            
            case 'wrong-structure':
                // Return valid JSON but wrong structure (not embeddings format)
                return [
                    'code' => 200,
                    'headers' => ['Content-Type: application/json'],
                    'body' => json_encode([
                        'result' => 'success',
                        'message' => 'This is not the expected embeddings format'
                    ])
                ];
            
            case 'default':
            default:
                // Fall through to normal JSON response
                break;
        }
    }
    
    // Generate deterministic random embeddings seeded by input text
    $embeddingsData = [];
    foreach ($inputData as $idx => $text) {
        // Clean text (remove delay/provider/response-type parameters) before generating embedding
        $cleanText = clean_input_text($text);
        // Use index as seed offset to ensure different texts get different embeddings
        $embedding = generate_embedding($cleanText, $dimensions, $idx * 1000);
        $embeddingsData[] = [
            'embedding' => $embedding
        ];
    }
    
    return [
        'code' => 200,
        'headers' => ['Content-Type: application/json'],
        'body' => json_encode([
            'data' => $embeddingsData,
            'model' => $model,
            'usage' => [
                'prompt_tokens' => 0,
                'total_tokens' => 0
            ]
        ])
    ];
}

/**
 * Show help message.
 */
function show_help() {
    echo "Mock Embeddings API Server\n";
    echo "==========================\n\n";
    echo "This server mimics OpenAI, Voyage, and Jina API endpoints and returns\n";
    echo "deterministic random embeddings seeded by input text.\n\n";
    echo "Usage:\n";
    echo "  php mock-embeddings-server.php [OPTIONS]\n\n";
    echo "Options:\n";
    echo "  --port PORT          Port to listen on (default: 8080)\n";
    echo "  --host HOST          Host to bind to (default: localhost)\n";
    echo "  --provider PROVIDER  Provider name: openai, voyage, or jina\n";
    echo "                       (validates model matches provider; omit for generic server)\n";
    echo "  --delay SECONDS      Delay in seconds before responding (default: 0.0)\n";
    echo "                       Simulates slow API responses\n";
    echo "  --help, -h           Show this help message\n\n";
    echo "Input Text Parameters:\n";
    echo "  The server also supports parameters embedded in the input text:\n";
    echo "  - delay=N            Override command line delay (e.g., \"delay=2.5\")\n";
    echo "  - provider=NAME      Override command line provider (e.g., \"provider=openai\")\n";
    echo "  - response-type=TYPE Override response format (e.g., \"response-type=html\")\n";
    echo "                       Valid types: html, invalid-json, wrong-structure, default\n";
    echo "                       These parameters are removed from text before generating embeddings\n\n";
    echo "Examples:\n";
    echo "  php mock-embeddings-server.php --port 8080 --provider openai\n";
    echo "  php mock-embeddings-server.php --port 8081 --provider voyage --delay 2.0\n";
    echo "  php mock-embeddings-server.php --port 8082\n\n";
    echo "Features:\n";
    echo "  - Deterministic embeddings (same text = same vector)\n";
    echo "  - API key validation (all keys valid except \"wrong\")\n";
    echo "  - Model/provider validation\n";
    echo "  - Configurable delay for timeout testing\n";
    echo "  - Correct dimensions for each model\n";
    exit(0);
}

/**
 * Parse HTTP request.
 *
 * @param string $request Raw HTTP request
 * @return array Parsed request with 'method', 'path', 'headers', and 'body'
 */
function parse_request($request) {
    $lines = explode("\r\n", $request);
    $requestLine = array_shift($lines);
    
    // Parse request line
    list($method, $path) = explode(' ', $requestLine, 3);
    
    // Parse headers
    $headers = [];
    $bodyStart = false;
    $body = '';
    
    foreach ($lines as $line) {
        if ($line === '') {
            $bodyStart = true;
            continue;
        }
        
        if ($bodyStart) {
            $body .= $line . "\r\n";
        } else {
            if (preg_match('/^([^:]+):\s*(.+)$/', $line, $matches)) {
                $headers[trim($matches[1])] = trim($matches[2]);
            }
        }
    }
    
    $body = rtrim($body, "\r\n");
    
    return [
        'method' => $method,
        'path' => $path,
        'headers' => $headers,
        'body' => $body
    ];
}

/**
 * Format HTTP response.
 *
 * @param int $code HTTP status code
 * @param array $headers HTTP headers
 * @param string $body Response body
 * @return string Formatted HTTP response
 */
function format_response($code, $headers, $body) {
    $statusText = [
        200 => 'OK',
        400 => 'Bad Request',
        401 => 'Unauthorized',
        404 => 'Not Found',
        405 => 'Method Not Allowed'
    ][$code] ?? 'Unknown';
    
    $response = "HTTP/1.1 {$code} {$statusText}\r\n";
    foreach ($headers as $header) {
        $response .= $header . "\r\n";
    }
    $response .= "\r\n";
    $response .= $body;
    
    return $response;
}

/**
 * Main function.
 */
function main() {
    global $argv;
    
    // Parse command line arguments
    $options = getopt('h', ['help', 'port:', 'host:', 'provider:', 'delay:']);
    
    // Show help if requested or no arguments
    if (isset($options['h']) || isset($options['help']) || count($argv) === 1) {
        show_help();
    }
    
    $port = (int)($options['port'] ?? 8080);
    $host = $options['host'] ?? 'localhost';
    $provider = $options['provider'] ?? null;
    $delay = (float)($options['delay'] ?? 0.0);
    
    // Validate provider if specified
    if ($provider !== null && !in_array($provider, ['openai', 'voyage', 'jina'])) {
        echo "Error: Invalid provider '{$provider}'. Must be: openai, voyage, or jina\n";
        exit(1);
    }
    
    // Create socket
    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($socket === false) {
        echo "Error: Failed to create socket\n";
        exit(1);
    }
    
    // Set socket options
    socket_set_option($socket, SOL_SOCKET, SO_REUSEADDR, 1);
    
    // Bind socket
    if (!socket_bind($socket, $host, $port)) {
        echo "Error: Failed to bind to {$host}:{$port}\n";
        exit(1);
    }
    
    // Listen
    if (!socket_listen($socket, 5)) {
        echo "Error: Failed to listen on socket\n";
        exit(1);
    }
    
    $providerInfo = $provider ? " ({$provider})" : "";
    $delayInfo = $delay > 0 ? " with {$delay}s delay" : "";
    echo "Mock embeddings server{$providerInfo}{$delayInfo} listening on http://{$host}:{$port}/v1/embeddings\n";
    echo "Press Ctrl+C to stop\n";
    
    // Main server loop
    while (true) {
        $client = socket_accept($socket);
        if ($client === false) {
            continue;
        }
        
        // Read request
        $request = '';
        while (true) {
            $data = socket_read($client, 4096);
            if ($data === false || strlen($data) === 0) {
                break;
            }
            $request .= $data;
            // Check if we've received the full request (ends with \r\n\r\n)
            if (strpos($request, "\r\n\r\n") !== false) {
                // Check if there's a Content-Length header
                if (preg_match('/Content-Length:\s*(\d+)/i', $request, $matches)) {
                    $contentLength = (int)$matches[1];
                    $headerEnd = strpos($request, "\r\n\r\n");
                    $bodyStart = $headerEnd + 4;
                    $bodyLength = strlen($request) - $bodyStart;
                    if ($bodyLength < $contentLength) {
                        // Need to read more
                        $remaining = $contentLength - $bodyLength;
                        $data = socket_read($client, $remaining);
                        if ($data !== false) {
                            $request .= $data;
                        }
                    }
                }
                break;
            }
        }
        
        // Parse request
        $parsed = parse_request($request);
        
        // Handle request
        if ($parsed['method'] === 'OPTIONS') {
            // CORS preflight
            $response = format_response(200, [
                'Access-Control-Allow-Origin: *',
                'Access-Control-Allow-Methods: POST, OPTIONS',
                'Access-Control-Allow-Headers: Content-Type, Authorization'
            ], '');
        } elseif ($parsed['method'] === 'POST' && $parsed['path'] === '/v1/embeddings') {
            $result = handle_embeddings_request($parsed['headers'], $parsed['body'], $delay, $provider);
            $response = format_response($result['code'], $result['headers'], $result['body']);
        } else {
            $response = format_response(404, ['Content-Type: application/json'], json_encode([
                'error' => [
                    'message' => 'Not Found',
                    'type' => 'invalid_request_error',
                    'code' => 'not_found'
                ]
            ]));
        }
        
        // Send response
        socket_write($client, $response);
        socket_close($client);
    }
    
    socket_close($socket);
}

// Run main function
main();

