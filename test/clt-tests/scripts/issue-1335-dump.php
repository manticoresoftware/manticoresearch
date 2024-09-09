<?php
if (count($argv) < 4) die("Usage: ".__FILE__." <batch size> <concurrency> <docs>\n");

// This function waits for an idle mysql connection for the $query, runs it and exits
function process($query) {
    global $all_links;
    global $requests;
    global $latencies;
    foreach ($all_links as $k=>$link) {
        if (@$requests[$k]) continue;
        mysqli_query($link, $query, MYSQLI_ASYNC);
        @$requests[$k] = microtime(true);
        return true;
    }
    do {
        $links = $errors = $reject = array();
        foreach ($all_links as $link) {
            $links[] = $errors[] = $reject[] = $link;
        }
        $count = @mysqli_poll($links, $errors, $reject, 0, 1000);
        if ($count > 0) {
            foreach ($links as $j=>$link) {
                $res = @mysqli_reap_async_query($links[$j]);
                foreach ($all_links as $i=>$link_orig) if ($all_links[$i] === $links[$j]) break;
                if ($link->error) {
                    echo "ERROR in '".substr($query, 0, 100)."...': {$link->error}\n";
                    if (!mysqli_ping($link)) {
                        echo "ERROR: mysql connection is down, removing it from the pool\n";
                        unset($all_links[$i]); // remove the original link from the pool
                        unset($requests[$i]); // and from the $requests too
                    }
                    return false;
                }
                if ($res === false and !$link->error) continue;
                if (is_object($res)) {
                    mysqli_free_result($res);
                }
                $latencies[] = microtime(true) - $requests[$i];
                $requests[$i] = microtime(true);
                mysqli_query($link, $query, MYSQLI_ASYNC); // making next query
                return true;
            }
        };
    } while (true);
    return true;
}

$t = microtime(true);
$all_links = [];
$requests = [];
$latencies = [];
$c = 0;
for ($i=0;$i<$argv[2];$i++) {
  $m = @mysqli_connect('127.0.0.1', '', '', '', 9306);
      if (mysqli_connect_error()) die("Cannot connect to Manticore\n");
      $all_links[] = $m;
  }

// init
mysqli_query($all_links[0], "drop table if exists test");
mysqli_query($all_links[0], "create table test(street text attribute indexed, country string)");

$batch = [];
$query_start = "insert into test(id, street, country) values ";

$addresses = [
  0 => '312 Walnut Rd',
  1 => '8865 Cherry St',
  2 => '163 Elm Dr',
  3 => '7011 Elm Blvd',
  4 => '7270 Elm Rd',
  5 => '7484 Cherry Rd',
  6 => '8926 Cherry Ln',
  7 => '9224 Cherry St',
  8 => '3629 Aspen Dr',
  9 => '8991 Elm Blvd',
  10 => '4009 Elm Ave',
  11 => '4633 Elm Rd',
  12 => '3553 Birch Ln',
  13 => '4297 Spruce Rd',
  14 => '9114 Pine Dr',
  15 => '1602 Spruce Dr',
  16 => '8748 Oak Rd',
  17 => '5829 Aspen Ave',
  18 => '3302 Cherry St',
  19 => '9555 Cedar Ave',
  20 => '2007 Cherry St',
  21 => '9149 Walnut Ave',
  22 => '2093 Cedar Dr',
  23 => '7751 Cedar Dr',
  24 => '7819 Pine Ave',
  25 => '3938 Walnut Rd',
  26 => '200 Birch Ln',
  27 => '531 Walnut Dr',
  28 => '8287 Maple Ln',
  29 => '1784 Maple St',
  30 => '4243 Oak Blvd',
  31 => '4707 Spruce Rd',
  32 => '4823 Aspen Rd',
  33 => '7204 Cedar Blvd',
  34 => '1846 Pine Rd',
  35 => '4349 Birch St',
  36 => '2203 Elm Ln',
  37 => '6662 Cherry Rd',
  38 => '8335 Pine Dr',
  39 => '3535 Aspen Rd',
  40 => '9510 Spruce Rd',
  41 => '8827 Pine Dr',
  42 => '7437 Elm Ave',
  43 => '4744 Elm Ln',
  44 => '8260 Cedar Ave',
  45 => '7822 Cedar Dr',
  46 => '3127 Elm Dr',
  47 => '9972 Elm Ln',
  48 => '5298 Oak Rd',
  49 => '4435 Maple Dr',
  50 => '5410 Oak St',
  51 => '6070 Aspen Rd',
  52 => '9395 Oak Ln',
  53 => '6880 Pine Rd',
  54 => '2933 Cherry St',
  55 => '6810 Walnut Blvd',
  56 => '8694 Aspen Ln',
  57 => '4065 Maple Dr',
  58 => '2760 Cedar Ln',
  59 => '1401 Spruce Ave',
  60 => '2880 Cherry Dr',
  61 => '3549 Maple Ln',
  62 => '8382 Aspen St',
  63 => '6983 Maple St',
  64 => '58 Maple St',
  65 => '9475 Spruce Blvd',
  66 => '838 Walnut Blvd',
  67 => '261 Aspen Dr',
  68 => '6823 Elm St',
  69 => '5082 Cherry Ave',
  70 => '4613 Cherry Ln',
  71 => '1293 Cherry Ave',
  72 => '7504 Walnut Blvd',
  73 => '2354 Cedar Ave',
  74 => '4660 Spruce Dr',
  75 => '8854 Pine Blvd',
  76 => '9415 Walnut Ln',
  77 => '1092 Walnut Ave',
  78 => '270 Elm Rd',
  79 => '7132 Elm Rd',
  80 => '5726 Oak Ave',
  81 => '670 Cedar Dr',
  82 => '7131 Pine Blvd',
  83 => '6232 Cedar Ave',
  84 => '854 Walnut Ave',
  85 => '9687 Aspen Rd',
  86 => '7133 Oak Ln',
  87 => '9689 Spruce Ave',
  88 => '558 Cedar St',
  89 => '8294 Birch Dr',
  90 => '238 Oak Rd',
  91 => '5756 Maple Rd',
  92 => '6474 Aspen Blvd',
  93 => '6252 Pine Ln',
  94 => '1815 Maple Blvd',
  95 => '461 Maple Ave',
  96 => '3957 Maple Rd',
  97 => '885 Elm Ave',
  98 => '2747 Oak Dr',
  99 => '2566 Cedar Rd'];

$countries = [
    'AF', // Afghanistan
    'AX', // Åland Islands
    'AL', // Albania
    'DZ', // Algeria
    'AS', // American Samoa
    'AD', // Andorra
    'AO', // Angola
    'AI', // Anguilla
    'AQ', // Antarctica
    'AG', // Antigua and Barbuda
    'AR', // Argentina
    'AM', // Armenia
    'AW', // Aruba
    'AU', // Australia
    'AT', // Austria
    'AZ', // Azerbaijan
    'BS', // Bahamas
    'BH', // Bahrain
    'BD', // Bangladesh
    'BB', // Barbados
    'BY', // Belarus
    'BE', // Belgium
    'BZ', // Belize
    'BJ', // Benin
    'BM', // Bermuda
    'BT', // Bhutan
    'BO', // Bolivia
    'BQ', // Bonaire, Sint Eustatius and Saba
    'BA', // Bosnia and Herzegovina
    'BW', // Botswana
    'BV', // Bouvet Island
    'BR', // Brazil
    'IO', // British Indian Ocean Territory
    'BN', // Brunei Darussalam
    'BG', // Bulgaria
    'BF', // Burkina Faso
    'BI', // Burundi
    'KH', // Cambodia
    'CM', // Cameroon
    'CA', // Canada
    'CV', // Cape Verde
    'KY', // Cayman Islands
    'CF', // Central African Republic
    'TD', // Chad
    'CL', // Chile
    'CN', // China
    'CX', // Christmas Island
    'CC', // Cocos (Keeling) Islands
    'CO', // Colombia
    'KM', // Comoros
    'CG', // Congo
    'CD', // Congo, the Democratic Republic of the
    'CK', // Cook Islands
    'CR', // Costa Rica
    'CI', // Côte d'Ivoire
    'HR', // Croatia
    'CU', // Cuba
    'CW', // Curaçao
    'CY', // Cyprus
    'CZ', // Czech Republic
    'DK', // Denmark
    'DJ', // Djibouti
    'DM', // Dominica
    'DO', // Dominican Republic
    'EC', // Ecuador
    'EG', // Egypt
    'SV', // El Salvador
    'GQ', // Equatorial Guinea
    'ER', // Eritrea
    'EE', // Estonia
    'ET', // Ethiopia
    'FK', // Falkland Islands (Malvinas)
    'FO', // Faroe Islands
    'FJ', // Fiji
    'FI', // Finland
    'FR', // France
    'GF', // French Guiana
    'PF', // French Polynesia
    'TF', // French Southern Territories
    'GA', // Gabon
    'GM', // Gambia
    'GE', // Georgia
    'DE', // Germany
    'GH', // Ghana
    'GI', // Gibraltar
    'GR', // Greece
    'GL', // Greenland
    'GD', // Grenada
    'GP', // Guadeloupe
    'GU', // Guam
    'GT', // Guatemala
    'GG', // Guernsey
    'GN', // Guinea
    'GW', // Guinea-Bissau
    'GY', // Guyana
    'HT', // Haiti
    'HM', // Heard Island and McDonald Islands
    'VA', // Holy See (Vatican City State)
    'HN', // Honduras
    'HK', // Hong Kong
    'HU', // Hungary
    'IS', // Iceland
    'IN', // India
    'ID', // Indonesia
    'IR', // Iran, Islamic Republic of
    'IQ', // Iraq
    'IE', // Ireland
    'IM', // Isle of Man
    'IL', // Israel
    'IT', // Italy
    'JM', // Jamaica
    'JP', // Japan
    'JE', // Jersey
    'JO', // Jordan
    'KZ', // Kazakhstan
    'KE', // Kenya
    'KI', // Kiribati
    'KP', // Korea, Democratic People's Republic of
    'KR', // Korea, Republic of
    'KW', // Kuwait
    'KG', // Kyrgyzstan
    'LA', // Lao People's Democratic Republic
    'LV', // Latvia
    'LB', // Lebanon
    'LS', // Lesotho
    'LR', // Liberia
    'LY', // Libya
    'LI', // Liechtenstein
    'LT', // Lithuania
    'LU', // Luxembourg
    'MO', // Macao
    'MK', // Macedonia, the former Yugoslav Republic of
    'MG', // Madagascar
    'MW', // Malawi
    'MY', // Malaysia
    'MV', // Maldives
    'ML', // Mali
    'MT', // Malta
    'MH', // Marshall Islands
    'MQ', // Martinique
    'MR', // Mauritania
    'MU', // Mauritius
    'YT', // Mayotte
    'MX', // Mexico
    'FM', // Micronesia, Federated States of
    'MD', // Moldova, Republic of
    'MC', // Monaco
    'MN', // Mongolia
    'ME', // Montenegro
    'MS', // Montserrat
    'MA', // Morocco
    'MZ', // Mozambique
    'MM', // Myanmar
    'NA', // Namibia
    'NR', // Nauru
    'NP', // Nepal
    'NL', // Netherlands
    'NC', // New Caledonia
    'NZ', // New Zealand
    'NI', // Nicaragua
    'NE', // Niger
    'NG', // Nigeria
    'NU', // Niue
    'NF', // Norfolk Island
    'MP', // Northern Mariana Islands
    'NO', // Norway
    'OM', // Oman
    'PK', // Pakistan
    'PW', // Palau
    'PS', // Palestine, State of
    'PA', // Panama
    'PG', // Papua New Guinea
    'PY', // Paraguay
    'PE', // Peru
    'PH', // Philippines
    'PN', // Pitcairn
    'PL', // Poland
    'PT', // Portugal
    'PR', // Puerto Rico
    'QA', // Qatar
    'RE', // Réunion
    'RO', // Romania
    'RU', // Russian Federation
    'RW', // Rwanda
    'BL', // Saint Barthélemy
    'SH', // Saint Helena, Ascension and Tristan da Cunha
    'KN', // Saint Kitts and Nevis
    'LC', // Saint Lucia
    'MF', // Saint Martin (French part)
    'PM', // Saint Pierre and Miquelon
    'VC', // Saint Vincent and the Grenadines
    'WS', // Samoa
    'SM', // San Marino
    'ST', // Sao Tome and Principe
    'SA', // Saudi Arabia
    'SN', // Senegal
    'RS', // Serbia
    'SC', // Seychelles
    'SL', // Sierra Leone
    'SG', // Singapore
    'SX', // Sint Maarten (Dutch part)
    'SK', // Slovakia
    'SI', // Slovenia
    'SB', // Solomon Islands
    'SO', // Somalia
    'ZA', // South Africa
    'GS', // South Georgia and the South Sandwich Islands
    'SS', // South Sudan
    'ES', // Spain
    'LK', // Sri Lanka
    'SD', // Sudan
    'SR', // Suriname
    'SJ', // Svalbard and Jan Mayen
    'SZ', // Swaziland
    'SE', // Sweden
    'CH', // Switzerland
    'SY', // Syrian Arab Republic
    'TW', // Taiwan, Province of China
    'TJ', // Tajikistan
    'TZ', // Tanzania, United Republic of
    'TH', // Thailand
    'TL', // Timor-Leste
    'TG', // Togo
    'TK', // Tokelau
    'TO', // Tonga
    'TT', // Trinidad and Tobago
    'TN', // Tunisia
    'TR', // Turkey
    'TM', // Turkmenistan
    'TC', // Turks and Caicos Islands
    'TV', // Tuvalu
    'UG', // Uganda
    'UA', // Ukraine
    'AE', // United Arab Emirates
    'GB', // United Kingdom
    'US', // United States
    'UM', // United States Minor Outlying Islands
    'UY', // Uruguay
    'UZ', // Uzbekistan
    'VU', // Vanuatu
    'VE', // Venezuela, Bolivarian Republic of
    'VN', // Vietnam
    'VG', // Virgin Islands, British
    'VI', // Virgin Islands, U.S.
    'WF', // Wallis and Futuna
    'EH', // Western Sahara
    'YE', // Yemen
    'ZM', // Zambia
    'ZW'  // Zimbabwe
];

srand(0);

$error = false;
while (count($all_links) and $c < $argv[3]) {
  $batch[] = "(".($c+1).",'".$addresses[rand(0, count($addresses) - 1)]."', '".$countries[rand(0, count($countries) - 1)]."')";
  $c++;
  if (count($batch) == $argv[1]) {
    if (!process($query_start.implode(',', $batch))) {
      $error = true;
      break;
    }
    $batch = [];
  }
}
// wait until all the workers finish
do {
  $links = $errors = $reject = array();
  foreach ($all_links as $link)  $links[] = $errors[] = $reject[] = $link;
  $count = @mysqli_poll($links, $errors, $reject, 0, 100);
} while (count($all_links) != count($links) + count($errors) + count($reject));

echo "finished inserting\n";
echo round($argv[3] / (microtime(true) - $t))." docs per sec\n";
