# Ukrainian lemmatizer asset builder

This directory builds a native `uk.pak` asset for `morphology='lemmatize_uk'`
and `morphology='lemmatize_uk_all'`.

## Host usage

Build directly from upstream dictionaries:

```bash
python3 build_uk_lemmatizer_asset.py --from-upstream uk.pak
```

Build from an existing local `pymorphy2` dictionary directory:

```bash
python3 build_uk_lemmatizer_asset.py --dict-dir /path/to/pymorphy2_dicts_uk/data uk.pak
```

## Docker usage

Build the image:

```bash
docker build --load -t manticore-uk-pak -f misc/morphology/Dockerfile misc/morphology
```

If your Docker setup uses the `docker-container` builder driver, `--load` is
required so the image becomes available to `docker run`.

Generate `uk.pak` into the current directory:

```bash
docker run --rm --mount type=bind,src="$(pwd)",dst=/out manticore-uk-pak
```

The default container command writes:

```text
./uk.pak
```

To use an already available dictionary directory instead of downloading from
upstream:

```bash
docker run --rm \
  --mount type=bind,src="$(pwd)",dst=/out \
  --mount type=bind,src="/path/to/pymorphy2_dicts_uk/data",dst=/dict,readonly \
  manticore-uk-pak \
  --dict-dir /dict /out/uk.pak
```
