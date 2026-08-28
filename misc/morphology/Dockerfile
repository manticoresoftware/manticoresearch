FROM python:3.11-slim

WORKDIR /opt/uklemmatizer

COPY build_uk_lemmatizer_asset.py /opt/uklemmatizer/build_uk_lemmatizer_asset.py

VOLUME ["/out"]

ENTRYPOINT ["python3", "/opt/uklemmatizer/build_uk_lemmatizer_asset.py"]
CMD ["--from-upstream", "/out/uk.pak"]
