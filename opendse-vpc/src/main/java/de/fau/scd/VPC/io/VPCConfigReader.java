// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.io;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.stream.StreamSource;
//import javax.xml.XMLConstants;
//import javax.xml.validation.Schema;
//import javax.xml.validation.SchemaFactory;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import de.fau.scd.VPC.io.VpcDTD;

import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;

/**
 * The {@code VPCConfigReader} reads an XML VPC configuration into a
 * {@code org.w3c.dom.Document} from an {@code InputStream} or file.
 *
 * @author Joachim Falk
 */
public class VPCConfigReader extends Common {

    /**
     * Read specification from a file.
     *
     * @param filename
     *            The name of the file.
     * @throws FormatErrorException
     */
    public VPCConfigReader(String filename) throws FileNotFoundException, FormatErrorException {
        this(new File(filename));
    }

    /**
     * Read specification from a file.
     *
     * @param file
     *            The file.
     * @throws FormatErrorException
     */
    public VPCConfigReader(File file) throws FileNotFoundException, FormatErrorException {
        this(new StreamSource(new FileInputStream(file), file.toURI().toASCIIString()));
    }

    /**
     * Read specification from an input stream.
     *
     * @param in
     *            The input stream.
     * @throws FormatErrorException
     */
    public VPCConfigReader(InputStream in) throws FormatErrorException {
        this(new StreamSource(in, "<input stream>"));
    }

    public org.w3c.dom.Document getDocument() {
        return doc;
    }

    public org.w3c.dom.Element getDocumentElement() {
        return doc.getDocumentElement();
    }

    /**
     * Read specification from an input stream.
     *
     * @param in
     *            The input stream.
     * @throws FormatErrorException
     */
    protected VPCConfigReader(StreamSource in) throws FormatErrorException {
        try {
//          String networkgraphXSDUrl = "networkgraph.xsd";
//          StreamSource sources[] = new StreamSource[] {
//                  new StreamSource(new StringReader(NetworkgraphXSD.text), networkgraphXSDUrl)
//              };
//          SchemaFactory factory =
//              SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
//          factory.setErrorHandler(new DOMErrorHandler());
//          Schema schema = factory.newSchema(sources);
//          assert schema != null : "Can't parse '" + networkgraphXSDUrl + "'!";
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setNamespaceAware(true);
            dbf.setValidating(true);
            // See https://xerces.apache.org/xerces2-j/features.html
            dbf.setFeature("http://xml.org/sax/features/namespaces", true);
            dbf.setFeature("http://xml.org/sax/features/validation", true);
            dbf.setFeature("http://apache.org/xml/features/validation/schema", false);
//          dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-dtd-grammar", false);
//          dbf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
//          dbf.setSchema(schema);
//          Property 'http://java.sun.com/xml/jaxp/properties/schemaLanguage' cannot be set
//          when a non-null Schema object has already been specified.
//          dbf.setAttribute(
//              "http://java.sun.com/xml/jaxp/properties/schemaLanguage",
//              XMLConstants.W3C_XML_SCHEMA_NS_URI);
            DocumentBuilder docBuilder = dbf.newDocumentBuilder();
            docBuilder.setErrorHandler(new DOMErrorHandler());
            docBuilder.setEntityResolver(new MyResolver());
            doc = docBuilder.parse(in.getInputStream(), in.getSystemId());
            doc.normalize();
        } catch (Exception ex) {
//          ex.printStackTrace(System.err);
            throw new FormatErrorException(ex.getMessage());
        }
    }

    private static class MyResolver implements EntityResolver {
        public InputSource resolveEntity(String publicId, String systemId) {
            // use the default behaviour
            InputSource retval = null;

//          System.err.println("publicId: "+publicId);
//          System.err.println("systemId: "+systemId);
            if (systemId.startsWith("file:")) {
                java.io.File path = new java.io.File(systemId.substring("file:".length()));

                String vpcDTDUrl = "vpc.dtd";

                if (path.getName().equals(vpcDTDUrl)) {
                    retval = new InputSource(new StringReader(VpcDTD.text));
                    retval.setSystemId("file:"+vpcDTDUrl);
                    return new InputSource(new StringReader(VpcDTD.text));
                } else {
                    retval = new InputSource(new StringReader(""));
                }
            }
            return retval;
        }
    }

    private static class DOMErrorHandler implements ErrorHandler {
        @Override
        public void warning(SAXParseException exception) throws SAXException {
            printError("Warning", exception);
        }

        @Override
        public void error(SAXParseException exception) throws SAXException {
            printError("Error", exception);
            throw exception;
        }

        @Override
        public void fatalError(SAXParseException exception) throws SAXException {
            printError("Fatal Error", exception);
            throw exception;
        }

        /** Prints the error message. */
        private void printError(String type, SAXParseException ex) {
            System.err.print("[");
            System.err.print(type);
            System.err.print("] ");
            String systemId = ex.getSystemId();
            if (systemId != null) {
//              int index = systemId.lastIndexOf('/');
//              if (index != -1)
//                  systemId = systemId.substring(index + 1);
                System.err.print(systemId);
            }
            System.err.print(':');
            System.err.print(ex.getLineNumber());
            System.err.print(':');
            System.err.print(ex.getColumnNumber());
            System.err.print(": ");
            System.err.print(ex.getMessage());
            System.err.println();
            System.err.flush();
        }
    }

    protected final org.w3c.dom.Document doc;
}
