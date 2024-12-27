from collections.abc import Iterator
from typing import IO

class Reader:
    def __init__(
        self,
        file: str | IO[str],
        delim: str = ",",
        quotechar: str | None = '"',
        escapechar: str | None = None,
        commentchar: str | None = "#",
    ) -> None: ...
    def __iter__(self) -> Iterator[list[str]]: ...
    def __next__(self) -> list[str]: ...
